use std::sync::{Arc, Mutex};
use async_trait::async_trait;
use duckdb::{params, Connection, Arrow};
use duckdb::arrow::record_batch::RecordBatch;
use convergence::protocol::{ErrorResponse, FieldDescription, SqlState};
use convergence::engine::{Portal, Engine};
use convergence::protocol_ext::DataRowBatch;
use convergence_arrow::table::record_batch_to_rows;
use sqlparser::ast::Statement;

pub struct DuckDBPortal {
    records: Arc<Vec<RecordBatch>>
}

#[async_trait]
impl Portal for DuckDBPortal {
	async fn fetch(&mut self, batch: &mut DataRowBatch) -> Result<(), ErrorResponse> {
		for arrow_batch in self.records.as_ref() {
			record_batch_to_rows(&arrow_batch, batch)?;
		}
		Ok(())
	}
}

pub struct DuckDBEngine {
    conn: Arc<Mutex<Connection>>
}

impl DuckDBEngine {
    pub fn new(conn: Connection) -> Self {
        Self { 
            conn: Arc::new( Mutex::new(conn)),
        }
    }
}

#[async_trait]
impl Engine for DuckDBEngine {
	type PortalType = DuckDBPortal;

	async fn prepare(&mut self, statement: &Statement) -> Result<Vec<FieldDescription>, ErrorResponse> {
        let query = statement.to_string();
        let conn = self.conn.try_lock().unwrap();
        let stmt = conn.prepare(&query).unwrap();

		// let plan = self.ctx.sql(&statement.to_string()).await.map_err(df_err_to_sql)?;
		// schema_to_field_desc(&plan.schema().clone().into())
        Err(ErrorResponse { sql_state: (), severity: (), message: () })
	}

	async fn create_portal(&mut self, statement: &Statement) -> Result<Self::PortalType, ErrorResponse> {
		// let df = self.ctx.sql(&statement.to_string()).await.map_err(df_err_to_sql)?;
		// Ok(DataFusionPortal { df })
	}
}
