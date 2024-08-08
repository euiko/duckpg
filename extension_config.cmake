# This file is included by DuckDB's build system. It specifies which extension to load

# allow to DuckDB load this custom directory for DuckPG extension header
include_directories(${CMAKE_CURRENT_LIST_DIR}/include/duckpg)

# Extension from this repo
duckdb_extension_load(duckdb_pgwire
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}
    LOAD_TESTS
)

# Any extra extensions that should be built
# e.g.: duckdb_extension_load(json)
