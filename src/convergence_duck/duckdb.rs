use std::sync::{Arc, Mutex};
use datafusion::sql::sqlparser::ast::Statement;
use duckdb::{params, Connection, Arrow};
use duckdb::arrow::record_batch::RecordBatch;
use convergence::protocol::{ErrorResponse, FieldDescription, SqlState};
use convergence::engine::{Portal, Engine};
use convergence::protocol_ext::DataRowBatch;
use super::table::{record_batch_to_rows, schema_to_field_desc};
use async_trait::async_trait;

pub struct DuckDBPortal {
    records: Vec<RecordBatch>
}

#[async_trait]
impl Portal for DuckDBPortal {
	async fn fetch(&mut self, batch: &mut DataRowBatch) -> Result<(), ErrorResponse> {
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

	async fn prepare(&mut self, statement: &Statement) -> Result<Vec<FieldDescription>, ErrorResponse> {
        let query = statement.to_string();
		println!("prepare {}", query);
        let conn = self.conn.lock().unwrap();
        let mut stmt = conn.prepare(&query).unwrap();
		println!("statement prepared");
		// execute the query
		stmt.raw_execute();
		let field_desc = schema_to_field_desc(&stmt.schema().clone())?;
		println!("field desc len {}", field_desc.len());
		Ok(field_desc)
	}

	async fn create_portal(&mut self, statement: &Statement) -> Result<Self::PortalType, ErrorResponse> {
		let query = statement.to_string();
		println!("create_portal {}", query);
		let conn = self.conn.lock().unwrap();
		let mut stmt = conn.prepare(&query).unwrap();
		let records: Vec<RecordBatch> = stmt.query_arrow([]).unwrap().collect();
		println!("records len {}", records.len());
		Ok(DuckDBPortal { records })
	}
}
