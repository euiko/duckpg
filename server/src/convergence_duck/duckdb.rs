use std::sync::{Arc, Mutex};
use sqlparser::ast::Statement;
use duckdb::{params, Connection, Arrow};
use duckdb::arrow::record_batch::RecordBatch;
use pg_wire::protocol::{ErrorResponse, FieldDescription};
use pg_wire::engine::{Portal, Engine};
use pg_wire::protocol_ext::DataRowBatch;
use super::table::{record_batch_to_rows, schema_to_field_desc};
use async_trait::async_trait;

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
    conn: Arc<Mutex<Connection>>
}

impl DuckDBEngine {
    pub fn new(conn: Connection) -> Self {
        Self { 
            conn: Arc::new( Mutex::new(conn) ),
        }
    }
}

#[async_trait]
impl Engine for DuckDBEngine {
	type PortalType = DuckDBPortal;

	async fn prepare(&mut self, _: &String) -> Result<Vec<FieldDescription>, ErrorResponse> {
        // DuckDB doesn't support multi-stage query execution
		// the field description will be set during fetch
		Ok(vec![])
	}

	async fn create_portal(&mut self, query: &String) -> Result<Self::PortalType, ErrorResponse> {
		let conn = self.conn.lock().unwrap();
		let mut stmt = conn.prepare(&query).unwrap();
		let records: Vec<RecordBatch> = stmt.query_arrow([]).unwrap().collect();
		let schema = stmt.schema();
		let fields = schema_to_field_desc(&schema)?;
		Ok(DuckDBPortal { fields, records })
	}
}
