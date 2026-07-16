#include <catch2/catch_test_macros.hpp>

#include "styles/css_parser.h"

#define TAG "[css_parser_tests]"

using MAP = std::map<std::string, std::string>;

MAP testParser(std::string input)
{
    auto parser = CSSParser(input);
    return parser.body();
}

TEST_CASE("CSSParser test empty", TAG)
{
    auto actual = testParser("");
    REQUIRE(actual.empty() == true);
}

TEST_CASE("CSSParser one tag selector", TAG)
{
    auto actual = testParser("color: red;");
    MAP expected = { {"color", "red"} };
    REQUIRE(actual == expected);
}

TEST_CASE("CSSParser two tag selectors", TAG)
{
    auto actual = testParser("color: red; background-color: #112233;");
    MAP expected = { {"color", "red"}, {"background-color", "#112233"} };
    REQUIRE(actual == expected);
}
