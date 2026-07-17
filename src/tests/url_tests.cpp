#include <catch2/catch_test_macros.hpp>
#include "url.h"

#define TAG "[url_tests]"

URL getTestURL()
{
    return URL("https://example.com/index.html");
}

TEST_CASE("resolve full url", TAG)
{
    auto url = getTestURL();
    auto actual = url.resolve("https://example.com/styles.css");
    auto expected = URL("https://example.com/styles.css");

    REQUIRE(expected == actual);
}

TEST_CASE("resolve host-relative url", TAG)
{
    auto url = getTestURL();
    auto actual = url.resolve("/styles.css");
    auto expected = URL("https://example.com/styles.css");

    REQUIRE(expected == actual);
}

TEST_CASE("resolve path-relative url", TAG)
{
    auto url = getTestURL();
    auto actual = url.resolve("styles.css");
    auto expected = URL("https://example.com/styles.css");

    REQUIRE(expected == actual);
}

TEST_CASE("resolve scheme-relative url", TAG)
{
    auto url = getTestURL();
    auto actual = url.resolve("//example.com/styles.css");
    auto expected = URL("https://example.com/styles.css");

    REQUIRE(expected == actual);
}
