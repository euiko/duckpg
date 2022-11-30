package main

import (
	"context"
	"database/sql"
	"errors"
	"log"
	"os"

	wire "github.com/jeroenrinzema/psql-wire"
	"github.com/lib/pq/oid"

	_ "github.com/marcboeker/go-duckdb"
)

const (
	invalidOid oid.Oid = 9999
)

func main() {
	var (
		dbPath string = "./test.db"
	)

	args := os.Args
	if len(args) > 1 {
		dbPath = args[1]
	}

	db, err := sql.Open("duckdb", dbPath)
	if err != nil {
		log.Fatal("failed to open db", err)
	}

	log.Println("db successfuly opened", db.Stats())
	wire.ListenAndServe("127.0.0.1:5432", func(ctx context.Context, query string, writer wire.DataWriter, parameters []string) error {
		var (
			table            wire.Columns
			tableInitialized bool
		)

		rows, err := db.QueryContext(ctx, query)
		if err != nil {
			return err
		}
		defer rows.Close()

		for rows.Next() {
			if !tableInitialized {
				table, err = tableFromRows(rows)
				if err != nil {
					return err
				}
				writer.Define(table)
			}

			row := scanRows(rows, table)
			if len(row) > 0 {
				err = rows.Scan(row...)
			}

			if len(row) > 0 && err == nil {
				err = writer.Row(row)
			}

			if err != nil {
				return err
			}
		}

		return writer.Complete("OK")
	})
}

func tableFromRows(rows *sql.Rows) (wire.Columns, error) {
	var result wire.Columns
	types, err := rows.ColumnTypes()
	if err != nil {
		return nil, err
	}

	result = make(wire.Columns, len(types))
	for i := range types {
		colType := types[i]
		name := colType.Name()
		oid := typeToOid(colType)

		if oid == invalidOid {
			return nil, errors.New("unsupported type")
		}

		result[i] = wire.Column{
			Table:        0,
			Name:         name,
			AttrNo:       0,
			Oid:          oid,
			Width:        0,
			TypeModifier: 0,
			Format:       0,
		}

	}

	return result, nil
}

func scanRows(rows *sql.Rows, table wire.Columns) []any {
	row := make([]any, len(table))

	for i := range row {
		col := table[i]

		switch col.Oid {
		case oid.T_text:
			var v string
			row[i] = &v
		case oid.T_int4:
			var v int32
			row[i] = &v
		}
	}

	return row
}

func typeToOid(colType *sql.ColumnType) oid.Oid {
	switch colType.ScanType().Name() {
	case "string":
		return oid.T_text
	case "int32":
		return oid.T_int4
	}

	return invalidOid
}
