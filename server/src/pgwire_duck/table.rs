//! Utilities for converting between DuckDB Arrow and Postgres formats.

use pg_wire::protocol::{DataTypeOid, ErrorResponse, FieldDescription, SqlState};
use pg_wire::protocol_ext::DataWriter;
use duckdb::arrow::array::{
	BooleanArray, Date32Array, Date64Array, Float16Array, Float32Array, Float64Array, Int16Array, Int32Array,
	Int64Array, Int8Array, StringArray, TimestampMicrosecondArray, TimestampMillisecondArray, TimestampNanosecondArray,
	TimestampSecondArray, UInt16Array, UInt32Array, UInt64Array, UInt8Array, Decimal128Array,
};
use duckdb::arrow::datatypes::{DataType, Schema, TimeUnit};
use duckdb::arrow::record_batch::RecordBatch;

macro_rules! array_cast {
	($arrtype: ident, $arr: expr) => {
		$arr.as_any().downcast_ref::<$arrtype>().expect("array cast failed")
	};
}

macro_rules! array_val {
	($arrtype: ident, $arr: expr, $idx: expr, $func: ident) => {
		array_cast!($arrtype, $arr).$func($idx)
	};
	($arrtype: ident, $arr: expr, $idx: expr) => {
		array_val!($arrtype, $arr, $idx, value)
	};
}

/// Writes the contents of an Arrow [RecordBatch] into a Postgres [DataRowBatch].
pub fn record_batch_to_rows(arrow_batch: &RecordBatch, pg_batch: &mut DataWriter) -> Result<(), ErrorResponse> {
	for row_idx in 0..arrow_batch.num_rows() {
		let mut row = pg_batch.create_row();
		for col_idx in 0..arrow_batch.num_columns() {
			let col = arrow_batch.column(col_idx);
			if col.is_null(row_idx) {
				row.write_null();
			} else {
				match col.data_type() {
					DataType::Boolean => row.write_bool(array_val!(BooleanArray, col, row_idx) as bool),
					DataType::Int8 => row.write_int2(array_val!(Int8Array, col, row_idx) as i16),
					DataType::Int16 => row.write_int2(array_val!(Int16Array, col, row_idx)),
					DataType::Int32 => row.write_int4(array_val!(Int32Array, col, row_idx)),
					DataType::Int64 => row.write_int8(array_val!(Int64Array, col, row_idx)),
					DataType::UInt8 => row.write_int2(array_val!(UInt8Array, col, row_idx) as i16),
					DataType::UInt16 => row.write_int2(array_val!(UInt16Array, col, row_idx) as i16),
					DataType::UInt32 => row.write_int4(array_val!(UInt32Array, col, row_idx) as i32),
					DataType::UInt64 => row.write_int8(array_val!(UInt64Array, col, row_idx) as i64),
					DataType::Float16 => row.write_float4(array_val!(Float16Array, col, row_idx).to_f32()),
					DataType::Float32 => row.write_float4(array_val!(Float32Array, col, row_idx)),
					DataType::Float64 => row.write_float8(array_val!(Float64Array, col, row_idx)),
					DataType::Decimal128(p, s) => row.write_float8(decimal128_into_f64(array_val!(Decimal128Array, col, row_idx), p.clone(), s.clone())),
					DataType::Utf8 => row.write_string(array_val!(StringArray, col, row_idx)),
					DataType::Date32 => {
						row.write_date(array_val!(Date32Array, col, row_idx, value_as_date).ok_or_else(|| {
							ErrorResponse::error(SqlState::INVALID_DATETIME_FORMAT, "unsupported date type")
						})?)
					}
					DataType::Date64 => {
						row.write_date(array_val!(Date64Array, col, row_idx, value_as_date).ok_or_else(|| {
							ErrorResponse::error(SqlState::INVALID_DATETIME_FORMAT, "unsupported date type")
						})?)
					}
					DataType::Timestamp(unit, None) => row.write_timestamp(
						match unit {
							TimeUnit::Second => array_val!(TimestampSecondArray, col, row_idx, value_as_datetime),
							TimeUnit::Millisecond => {
								array_val!(TimestampMillisecondArray, col, row_idx, value_as_datetime)
							}
							TimeUnit::Microsecond => {
								array_val!(TimestampMicrosecondArray, col, row_idx, value_as_datetime)
							}
							TimeUnit::Nanosecond => {
								array_val!(TimestampNanosecondArray, col, row_idx, value_as_datetime)
							}
						}
						.ok_or_else(|| {
							ErrorResponse::error(SqlState::INVALID_DATETIME_FORMAT, "unsupported timestamp type")
						})?,
					),
					other => {
						return Err(ErrorResponse::error(
							SqlState::FEATURE_NOT_SUPPORTED,
							format!("arrow to pg conversion not implemented for {}", other),
						))
					}
				};
			}
		}
	}

	Ok(())
}

/// Converts an Arrow [DataType] into a Postgres [DataTypeOid].
pub fn data_type_to_oid(ty: &DataType) -> Result<DataTypeOid, ErrorResponse> {
	Ok(match ty {
		DataType::Boolean => DataTypeOid::Bool,
		DataType::Int8 | DataType::Int16 => DataTypeOid::Int2,
		DataType::Int32 => DataTypeOid::Int4,
		DataType::Int64 => DataTypeOid::Int8,
		// TODO: need to figure out a sensible mapping for unsigned
		DataType::UInt8 | DataType::UInt16 => DataTypeOid::Int2,
		DataType::UInt32 => DataTypeOid::Int4,
		DataType::UInt64 => DataTypeOid::Int8,
		DataType::Float16 | DataType::Float32 => DataTypeOid::Float4,
		DataType::Float64 => DataTypeOid::Float8,
		DataType::Decimal128(_,_) => DataTypeOid::Float8,
		DataType::Utf8 => DataTypeOid::Text,
		DataType::Date32 | DataType::Date64 => DataTypeOid::Date,
		DataType::Timestamp(_, None) => DataTypeOid::Timestamp,
		other => {
			return Err(ErrorResponse::error(
				SqlState::FEATURE_NOT_SUPPORTED,
				format!("arrow to pg conversion not implemented for {}", other),
			))
		}
	})
}

/// Converts an Arrow [Schema] into a vector of Postgres [FieldDescription] instances.
pub fn schema_to_field_desc(schema: &Schema) -> Result<Vec<FieldDescription>, ErrorResponse> {
	schema
		.fields()
		.iter()
		.map(|f| {
			Ok(FieldDescription {
				name: f.name().clone(),
				data_type: data_type_to_oid(f.data_type())?,
			})
		})
		.collect()
}

pub fn decimal128_into_f64(val: i128, _precision: u8, scale: u8) -> f64 {
	let mul = (10 as i64).pow(scale as u32) as i128;
	let num = val / mul;
	let rounded  = num * mul;
	let left_over = (val - rounded) as f64;
	let fraction = left_over/mul as f64;
	let result = (num as f64) + fraction;
	result
} 
