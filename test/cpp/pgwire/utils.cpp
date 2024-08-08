#include <catch2/catch.hpp>

#include <pgwire/utils.hpp>

using namespace pgwire;

TEST_CASE( "String formatting utility ", "[string_utils]" ) {
    REQUIRE( string_format("Hello world") == "Hello world" );
    REQUIRE( string_format("Hello %s", "John Doe") == "Hello John Doe" );
    REQUIRE( string_format("Welcome %s, today you are %d years old", "John", 25) == "Welcome John, today you are 25 years old" );
}
