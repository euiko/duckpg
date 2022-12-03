use std::path::Path;
use duckdb::{DuckdbConnectionManager};
use duckdb::arrow::record_batch::RecordBatch;
use pg_wire::protocol::{ErrorResponse, FieldDescription, SqlState};
use pg_wire::engine::{Portal, Engine};
use pg_wire::protocol_ext::DataRowBatch;
use r2d2::Pool;
use super::table::{record_batch_to_rows, schema_to_field_desc};
use async_trait::async_trait;

#[derive(Debug)]
pub enum EngineError {
	PoolError(r2d2::Error),
	DuckdbError(duckdb::Error),
}


pub struct DuckDBPortal {
	fields: Vec<FieldDescription>,
    records: Vec<RecordBatch>,
}

#[async_trait]
impl Portal for DuckDBPortal {
	async fn fetch(&mut self, batch: &mut DataRowBatch) -> Result<(), ErrorResponse> {
		batch.set_fields(self.fields.to_owned());
		for arrow_batch in &self.records {
			record_batch_to_rows(&arrow_batch, batch)?;
		}
		Ok(())
	}
}

pub struct DuckDBEngine {
    pool: Pool<DuckdbConnectionManager>
}

impl DuckDBEngine {
	pub fn new(manager: DuckdbConnectionManager) -> Result<Self, EngineError> {
		let pool = Pool::builder()
			.min_idle(Some(4))
			.max_size(16)
			.build(manager)
			.map_err(EngineError::PoolError)?;
		Ok(Self{ pool })
	}
	
    pub fn file<P: AsRef<Path>>(path: P) -> Result<Self, EngineError> {
		let manager = DuckdbConnectionManager::file(path).map_err(EngineError::DuckdbError)?;
		Self::new(manager)
    }
}

#[async_trait]
impl Engine for DuckDBEngine {
	type PortalType = DuckDBPortal;

	async fn prepare(&mut self, _: &String) -> Result<Vec<FieldDescription>, ErrorResponse> {
        // DuckDB doesn't support multi-stage query execution
		// the field description will be set during Portal's fetch
		Ok(vec![])
	}

	async fn create_portal(&mut self, query: &String) -> Result<Self::PortalType, ErrorResponse> {
		let conn = self.pool.get()
			.map_err(|e| ErrorResponse::error(
				SqlState::CONNECTION_EXCEPTION, 
				format!("cannot retrieve lock to connection : {}", e.to_string()), 
			))?;

		let mut stmt = conn.prepare(&query).map_err(to_wire_error)?;
		let records: Vec<RecordBatch> = stmt.query_arrow([])
			.map_err(to_wire_error)?
			.collect();
		let schema = stmt.schema();
		let fields = schema_to_field_desc(&schema)?;
		Ok(DuckDBPortal { fields, records })
	}
}

fn to_wire_error(e: duckdb::Error) -> ErrorResponse {
	let state = match e {
				duckdb::Error::DuckDBFailure(_, _) => SqlState::CONNECTION_EXCEPTION, 
				duckdb::Error::FromSqlConversionFailure(_, _, _) => SqlState::SYNTAX_ERROR,
				duckdb::Error::IntegralValueOutOfRange(_, _) => SqlState::DATA_EXCEPTION,
				duckdb::Error::Utf8Error(_) => SqlState::DATA_EXCEPTION,
				duckdb::Error::NulError(_) => SqlState::DATA_EXCEPTION,
				duckdb::Error::InvalidParameterName(_) => SqlState::SYNTAX_ERROR,
				duckdb::Error::InvalidPath(_) => SqlState::SYNTAX_ERROR,
				duckdb::Error::ExecuteReturnedResults => SqlState::DATA_EXCEPTION,
				duckdb::Error::QueryReturnedNoRows => SqlState::DATA_EXCEPTION,
				duckdb::Error::InvalidColumnIndex(_) => SqlState::DATA_EXCEPTION,
				duckdb::Error::InvalidColumnName(_) => SqlState::DATA_EXCEPTION,
				duckdb::Error::InvalidColumnType(_, _, _) => SqlState::DATA_EXCEPTION,
				duckdb::Error::StatementChangedRows(_) => SqlState::DATA_EXCEPTION,
				duckdb::Error::ToSqlConversionFailure(_) => SqlState::SYNTAX_ERROR,
				duckdb::Error::InvalidQuery => SqlState::SYNTAX_ERROR,
				duckdb::Error::MultipleStatement => SqlState::SYNTAX_ERROR,
				duckdb::Error::InvalidParameterCount(_, _) => SqlState::SYNTAX_ERROR,
				duckdb::Error::AppendError => SqlState::DATA_EXCEPTION,
				_ => todo!()
	};
	ErrorResponse::error(
		state,
		e.to_string(),
	)
}