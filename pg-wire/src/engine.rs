//! Contains core interface definitions for custom SQL engines.

use crate::protocol::{ErrorResponse, FieldDescription, Startup};
use crate::protocol_ext::DataWriter;
use async_trait::async_trait;

/// A Postgres portal. Portals represent a prepared statement with all parameters specified.
///
/// See Postgres' protocol docs regarding the [extended query overview](https://www.postgresql.org/docs/current/protocol-overview.html#PROTOCOL-QUERY-CONCEPTS)
/// for more details.
#[async_trait]
pub trait Portal: Send + Sync {
	/// Fetches the contents of the portal into a [DataRowBatch].
	async fn fetch(&mut self, w: &mut DataWriter) -> Result<(), ErrorResponse>;
}

/// The engine trait is the core of the `convergence` crate, and is responsible for dispatching most SQL operations.
///
/// Each connection is allocated an [Engine] instance, which it uses to prepare statements, create portals, etc.
#[async_trait]
pub trait Engine: Send + Sync + 'static {
	/// The [Portal] implementation used by [Engine::create_portal].
	type PortalType: Portal;

	/// Prepare the engine during Startup state of the postgresql conenection
	async fn startup(&mut self) -> Result<(), ErrorResponse>;

	/// Prepares a statement, returning a vector of field descriptions for the final statement result.
	async fn prepare(&mut self, query: &String) -> Result<Vec<FieldDescription>, ErrorResponse>;

	/// Creates a new portal for the given String.
	async fn create_portal(&mut self, query: &String) -> Result<Self::PortalType, ErrorResponse>;
}
