mod convergence_duck;

use std::env;
use std::sync::Arc;
use convergence::server::{self, BindOptions};
use convergence_duck::duckdb::DuckDBEngine;
use duckdb::{params, Connection, Result};
use duckdb::arrow::record_batch::RecordBatch;
use duckdb::arrow::util::pretty::print_batches;


async fn new_engine(db_path: String) -> DuckDBEngine {
    let conn = Connection::open(db_path).unwrap();
    DuckDBEngine::new(conn)
}

#[tokio::main]
async fn main() {
    let args: Vec<String> = env::args().collect();
    let mut db_path = String::from("./db.duckdb");
    if args.len() > 1 {
        db_path = String::from(&args[1]);
    }

	let port = server::run(BindOptions::new().with_port(5433), Arc::new(move || Box::pin(new_engine(db_path.clone()))))
		.await
		.unwrap();
}
