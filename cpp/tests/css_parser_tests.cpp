#include <catch2/catch_test_macros.hpp>

#include "styles/css_parser.h"

TEST_CASE("CSSParser test empty")
{
    auto parser = CSSParser("");
    auto body = parser.body();
    REQUIRE(body.empty() == true);
}

// TODO: write more tests....