# DuckPG - DuckDB extension with postgresql wire protocol enabled

This extension is used for experimentation of adding PostgreSQL wire protocol to the DuckDB ecosystem through an extension named `duckdb_pgwire`.

The PostgreSQL wire protocol is implemented in standalone component/library named `pgwire` that heavily inspired by https://github.com/returnString/convergence and https://github.com/jeroenrinzema/psql-wire.

## Milestone

- [x] Simple query support
- [x] DuckDB extension
- [x] Simple golang client
- [ ] PGWire unit tests
- [ ] Support more data type
- [ ] Logging
- [ ] Configuration
- [ ] Session Manager
- [ ] Extended Query
- [ ] So on...

## Building and Running

Clone the repository along with the submodule with :
```bash
git clone --recurse-submodules https://github.com/euiko/duckpg.git
```

Build the extension
```bash
make -j$(nproc)
```

Open the duckdb shell or through any duckdb embedded client and load the extension
```bash
# example usage with cli
cd build/release
# somehow still require manual loading of the extension even already built onto the duckdb shell/cli
./duckdb -cmd 'load duckdb_pgwire'   

```

And on the other terminal you can use psql and connect to port 15432 with ssl disabled
```bash
psql 'postgresql://localhost:15432/main' -c 'select * from generate_series(0, 100)'
```

Or you can use the postgresql driver in your language choice.
You can also run sample client in golang provided in this repo
```bash
# from the root directory
cd client/go/cmd/simple
go build && ./simple
```