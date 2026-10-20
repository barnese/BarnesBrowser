#include <catch2/catch_test_macros.hpp>
#include "networking/header_parser_helpers.h"

#define TAG "[test_header_parser_helpers]"

TEST_CASE("parse 200 status code from status line", TAG)
{
    std::string input = "HTTP/1.1 200 OK";

    auto expected = "200";
    auto actual = parseStatusCode(input);

    REQUIRE(expected == actual);
}

TEST_CASE("parse 404 status code from status line", TAG)
{
    std::string input = "HTTP/1.1 404 Not Found";

    auto expected = "404";
    auto actual = parseStatusCode(input);

    REQUIRE(expected == actual);
}

TEST_CASE("parse Cache-Control with no-store value", TAG)
{
    std::string input = "no-store";

    auto expected = CachePolicy { true, std::nullopt };
    auto actual = parseCacheControl(input);

    REQUIRE(expected == actual);
}

TEST_CASE("parse Cache-Control with max-age", TAG)
{
    std::string input = "max-age=60";

    auto expected = CachePolicy { false, std::chrono::seconds(60) };
    auto actual = parseCacheControl(input);

    REQUIRE(expected == actual);
}

TEST_CASE("parse Cache-Control with max-age and other values", TAG)
{
    std::string input = "public, max-age=60, must-revalidate";

    auto expected = CachePolicy { false, std::chrono::seconds(60) };
    auto actual = parseCacheControl(input);

    REQUIRE(expected == actual);
}

TEST_CASE("parse Cache-Control with unsupported values", TAG)
{
    std::string input = "private";

    auto expected = std::nullopt;
    auto actual = parseCacheControl(input);

    REQUIRE(expected == actual);
}
