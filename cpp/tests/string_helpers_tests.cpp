#include <catch2/catch_test_macros.hpp>
#include "helpers/string_helpers.h"

#define TAG "[string_helpers_tests]"

TEST_CASE("split() splits a string by the delimiter", TAG)
{
    std::vector<std::string> expected {"a", "b", "c"};
    auto actual = split("a b c", " ", false);
    REQUIRE(expected == actual);
}

TEST_CASE("split() only splits the first string by the delimiter", TAG)
{
    std::vector<std::string> expected {"a", "b c"};
    auto actual = split("a b c", " ", true);
    REQUIRE(expected == actual);
}

TEST_CASE("trimRight() removes whitespace to the right of a string", TAG)
{
    REQUIRE(trimRight("hi ") == "hi");
    REQUIRE(trimRight(" hi ") == " hi");
    REQUIRE(trimRight("hi   \n") == "hi");
}

TEST_CASE("trimLeft() removes whitespace to the left of a string", TAG)
{
    REQUIRE(trimLeft(" hi") == "hi");
    REQUIRE(trimLeft("     hi") == "hi");
    REQUIRE(trimLeft("     hi  ") == "hi  " );
}

TEST_CASE("trim() removes whitespace around a string", TAG)
{
    REQUIRE(trim("  hello  \n") == "hello");
}

TEST_CASE("toLowercase() converts text to lowercase", TAG)
{
    REQUIRE(toLowercase("Hello")== "hello");
}
