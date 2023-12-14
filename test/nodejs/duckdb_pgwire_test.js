var duckdb = require('../../duckdb/tools/nodejs');
var assert = require('assert');

describe(`duckdb_pgwire extension`, () => {
    let db;
    let conn;
    before((done) => {
        db = new duckdb.Database(':memory:', {"allow_unsigned_extensions":"true"});
        conn = new duckdb.Connection(db);
        conn.exec(`LOAD '${process.env.DUCKDB_PGWIRE_EXTENSION_BINARY_PATH}';`, function (err) {
            if (err) throw err;
            done();
        });
    });

    it('duckdb_pgwire function should return expected string', function (done) {
        db.all("SELECT duckdb_pgwire('Sam') as value;", function (err, res) {
            if (err) throw err;
            assert.deepEqual(res, [{value: "DuckdbPgwire Sam 🐥"}]);
            done();
        });
    });

    it('duckdb_pgwire_openssl_version function should return expected string', function (done) {
        db.all("SELECT duckdb_pgwire_openssl_version('Michael') as value;", function (err, res) {
            if (err) throw err;
            assert(res[0].value.startsWith('DuckdbPgwire Michael, my linked OpenSSL version is OpenSSL'));
            done();
        });
    });
});