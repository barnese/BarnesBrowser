#include <catch2/catch_test_macros.hpp>
#include "styles/css_parser.h"

#define TAG "[css_parser_tests]"

using Attributes = std::map<std::string, std::string>;
using ComparableRules = std::map<std::string, Attributes>;

ComparableRules makeComparable(const CSSRules& rules)
{
    ComparableRules comparable;

    for (const auto& [name, rule] : rules)
    {        
        comparable.emplace(rule.node->getName(), rule.styles);
    }

    return comparable;
}

CSSRules testRulesParser(std::string input)
{
    auto parser = CSSParser(input);
    return parser.parse();
}

TEST_CASE("CSSParser test empty", TAG)
{
    auto actual = CSSParser("").body();
    REQUIRE(actual.empty() == true);
}

TEST_CASE("CSSParser one attribute pair", TAG)
{
    auto actual = CSSParser("color: red;").body();
    Attributes expected = { {"color", "red"} };
    REQUIRE(actual == expected);
}

TEST_CASE("CSSParser two attribute pairs", TAG)
{
    auto actual = CSSParser("color: red; background-color: #112233;").body();
    Attributes expected = { {"color", "red"}, {"background-color", "#112233"} };
    REQUIRE(actual == expected);
}

TEST_CASE("CSSParser should parse one selector with one attribute pair", TAG)
{
    std::string input = "p { color: red; }\n";
    auto actual = CSSParser(input).parse();
    ComparableRules expected { { "p", { { "color", "red" } } } };
    REQUIRE(makeComparable(actual) == expected);
}

TEST_CASE("CSSParser should ignore comments", TAG)
{
    std::string input = "/* a comment */\np { color: red; }\n";
    auto actual = CSSParser(input).parse();
    ComparableRules expected { { "p", { { "color", "red" } } } };
    REQUIRE(makeComparable(actual) == expected);
}
