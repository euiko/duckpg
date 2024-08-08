PROJ_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# Configuration of extension
EXT_NAME=duckdb_pgwire
EXT_CONFIG=${PROJ_DIR}extension_config.cmake
EXT_FLAGS=-DINCLUDE_DIRECTORIES=${PROJ_DIR}include/duckpg

# Include the Makefile from extension-ci-tools
include makefiles/duckdb_extension.Makefile
