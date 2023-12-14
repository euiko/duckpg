package main

import (
	"database/sql"
	"log"
	"os"

	_ "github.com/lib/pq"
)

func main() {
	dbUri := "postgresql://localhost:15432/main?sslmode=disable"
	if v := os.Getenv("DB_URI"); v != "" {
		dbUri = v
	}

	db, err := sql.Open("postgres", dbUri)
	if err != nil {
		log.Fatalln("failed open db, err:")
	}
	defer db.Close()

	if _, err := db.Exec("CREATE TABLE users(name varchar)"); err != nil {
		log.Fatalln("failed to create table, err:", err)
	}

	if _, err := db.Exec("INSERT INTO users(name) select 'euiko' from generate_series(0, 10)"); err != nil {
		log.Fatalln("failed to insert data, err:", err)
	}

	rows, err := db.Query("SELECT * FROM users")
	if err != nil {
		panic(err)
	}

	for rows.Next() {
		var name string
		if err := rows.Scan(&name); err != nil {
			log.Println("cannot scan value with err:", err)
			continue
		}

		log.Println("name:", name)
	}

}
