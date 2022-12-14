//! Contains the [Connection] struct, which represents an individual Postgres session, and related types.

use crate::engine::{Engine, Portal};
use crate::protocol::*;
use crate::protocol_ext::DataWriter;
use futures::{SinkExt, StreamExt};
use std::collections::HashMap;
use tokio::io::{AsyncRead, AsyncWrite};
use tokio_util::codec::Framed;

/// Describes an error that may or may not result in the termination of a connection.
#[derive(thiserror::Error, Debug)]
pub enum ConnectionError {
	/// A protocol error was encountered, e.g. an invalid message for a connection's current state.
	#[error("protocol error: {0}")]
	Protocol(#[from] ProtocolError),
	/// A Postgres error containing a SqlState code and message occurred.
	/// May result in connection termination depending on the severity.
	#[error("error response: {0}")]
	ErrorResponse(#[from] ErrorResponse),
	/// The connection was closed.
	/// This always implies connection termination.
	#[error("connection closed")]
	ConnectionClosed,
}

#[derive(Debug)]
enum ConnectionState {
	Startup,
	Idle,
}

#[derive(Debug, Clone)]
struct PreparedStatement {
	pub statement: String,
	pub fields: Vec<FieldDescription>,
}

struct BoundPortal<E: Engine> {
	pub portal: E::PortalType,
	pub row_desc: RowDescription,
}

/// Describes a connection using a specific engine.
/// Contains connection state including prepared statements and portals.
pub struct Connection<E: Engine> {
	engine: E,
	state: ConnectionState,
	statements: HashMap<String, PreparedStatement>,
	portals: HashMap<String, Option<BoundPortal<E>>>,
}

impl<E: Engine> Connection<E> {
	/// Create a new connection from an engine instance.
	pub fn new(engine: E) -> Self {
		Self {
			state: ConnectionState::Startup,
			statements: HashMap::new(),
			portals: HashMap::new(),
			engine,
		}
	}

	fn prepared_statement(&self, name: &str) -> Result<&PreparedStatement, ConnectionError> {
		Ok(self
			.statements
			.get(name)
			.ok_or_else(|| ErrorResponse::error(SqlState::INVALID_SQL_STATEMENT_NAME, "missing statement"))?)
	}

	fn portal(&self, name: &str) -> Result<&Option<BoundPortal<E>>, ConnectionError> {
		Ok(self
			.portals
			.get(name)
			.ok_or_else(|| ErrorResponse::error(SqlState::INVALID_CURSOR_NAME, "missing portal"))?)
	}

	fn portal_mut(&mut self, name: &str) -> Result<&mut Option<BoundPortal<E>>, ConnectionError> {
		Ok(self
			.portals
			.get_mut(name)
			.ok_or_else(|| ErrorResponse::error(SqlState::INVALID_CURSOR_NAME, "missing portal"))?)
	}

	async fn step(
		&mut self,
		framed: &mut Framed<impl AsyncRead + AsyncWrite + Unpin, ConnectionCodec>,
	) -> Result<Option<ConnectionState>, ConnectionError> {
		match self.state {
			ConnectionState::Startup => {
				match framed.next().await.ok_or(ConnectionError::ConnectionClosed)?? {
					ClientMessage::Startup(_startup) => {
						// do startup stuff
					}
					ClientMessage::SSLRequest => {
						// we don't support SSL for now
						// client will retry with startup packet
						framed.send('N').await?;
						return Ok(Some(ConnectionState::Startup));
					}
					_ => {
						return Err(
							ErrorResponse::fatal(SqlState::PROTOCOL_VIOLATION, "expected startup message").into(),
						)
					}
				}

				framed.send(AuthenticationOk).await?;

				let param_statuses = &[
					("server_version", "13"),
					("server_encoding", "UTF8"),
					("client_encoding", "UTF8"),
					("DateStyle", "ISO"),
					("TimeZone", "UTC"),
					("integer_datetimes", "on"),
				];

				for &(param, status) in param_statuses {
					framed.send(ParameterStatus::new(param, status)).await?;
				}

				// startup the engine before ready to be queried
				self.engine.startup().await?;

				framed.send(ReadyForQuery).await?;

				Ok(Some(ConnectionState::Idle))
			}
			ConnectionState::Idle => {
				match framed.next().await.ok_or(ConnectionError::ConnectionClosed)?? {
					ClientMessage::Parse(parse) => {
						self.statements.insert(
							parse.prepared_statement_name,
							PreparedStatement {
								fields: self.engine.prepare(&parse.query).await?,
								statement: parse.query,
							},
						);
						framed.send(ParseComplete).await?;
					}
					ClientMessage::Bind(bind) => {
						let format_code = match bind.result_format {
							BindFormat::All(format) => format,
							BindFormat::PerColumn(_) => {
								return Err(ErrorResponse::error(
									SqlState::FEATURE_NOT_SUPPORTED,
									"per-column format codes not supported",
								)
								.into());
							}
						};

						let prepared = self.prepared_statement(&bind.prepared_statement_name)?.clone();
						let portal = self.engine.create_portal(&prepared.statement).await?;
						let row_desc = RowDescription {
							fields: prepared.fields.clone(),
							format_code,
						};
						let portal = Some(BoundPortal { portal, row_desc });

						self.portals.insert(bind.portal, portal);

						framed.send(BindComplete).await?;
					}
					ClientMessage::Describe(Describe::PreparedStatement(ref statement_name)) => {
						let fields = self.prepared_statement(statement_name)?.fields.clone();
						framed.send(ParameterDescription {}).await?;
						framed
							.send(RowDescription {
								fields,
								format_code: FormatCode::Text,
							})
							.await?;
					}
					ClientMessage::Describe(Describe::Portal(ref portal_name)) => match self.portal(portal_name)? {
						Some(portal) => framed.send(portal.row_desc.clone()).await?,
						None => framed.send(NoData).await?,
					},
					ClientMessage::Sync => {
						framed.send(ReadyForQuery).await?;
					}
					ClientMessage::Execute(exec) => match self.portal_mut(&exec.portal)? {
						Some(bound) => {
							let row_desc = bound.row_desc.clone();
							let mut writer = DataWriter::new(row_desc);
							bound.portal.fetch(&mut writer).await?;
							let num_rows = writer.num_rows();

							framed.send(writer).await?;

							framed
								.send(CommandComplete {
									command_tag: format!("SELECT {}", num_rows),
								})
								.await?;
						}
						None => {
							framed.send(EmptyQueryResponse).await?;
						}
					},
					ClientMessage::Query(query) => {
						let fields = self.engine.prepare(&query).await?;
							let row_desc = RowDescription {
								fields,
								format_code: FormatCode::Text,
							};
							let mut portal = self.engine.create_portal(&query).await?;

							let mut writer = DataWriter::new(row_desc);
							portal.fetch(&mut writer).await?;
							let num_rows = writer.num_rows();

							framed.send(writer.row_desc().clone()).await?;
							framed.send(writer).await?;

							framed
								.send(CommandComplete {
									command_tag: format!("SELECT {}", num_rows),
								})
								.await?;
						framed.send(ReadyForQuery).await?;
					}
					ClientMessage::Terminate => return Ok(None),
					_ => return Err(ErrorResponse::error(SqlState::PROTOCOL_VIOLATION, "unexpected message").into()),
				};

				Ok(Some(ConnectionState::Idle))
			}
		}
	}

	/// Given a stream (typically TCP), extract Postgres protocol messages and respond accordingly.
	/// This function only returns when the connection is closed (either gracefully or due to an error).
	pub async fn run(&mut self, stream: impl AsyncRead + AsyncWrite + Unpin) -> Result<(), ConnectionError> {
		let mut framed = Framed::new(stream, ConnectionCodec::new());
		loop {
			let new_state = match self.step(&mut framed).await {
				Ok(Some(state)) => state,
				Ok(None) => return Ok(()),
				Err(ConnectionError::ErrorResponse(err_info)) => {
					framed.send(err_info.clone()).await?;

					if err_info.severity == Severity::FATAL {
						return Err(err_info.into());
					}

					framed.send(ReadyForQuery).await?;
					ConnectionState::Idle
				}
				Err(err) => {
					framed
						.send(ErrorResponse::fatal(SqlState::CONNECTION_EXCEPTION, "connection error"))
						.await?;
					return Err(err);
				}
			};

			self.state = new_state;
		}
	}
}
