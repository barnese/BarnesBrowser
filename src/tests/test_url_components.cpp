#include <catch2/catch_test_macros.hpp>
#include "networking/url_components.h"

#include <iostream>

#define TAG "[test_url_components]"

// Helper for debugging URL components
void printUrlComponents(const std::string& name, const UrlComponents& components)
{
    std::cout << "name = " << name
              << ", scheme = " << components.scheme
              << ", host = " << components.host
              << ", path = " << components.path
              << ", port = " << components.port
              << ", base64 = " << components.base64
              << ", mediaType = " << (components.mediaType.has_value() ? *components.mediaType : "none")
              << ", data = " << (components.data.has_value() ? *components.data : "none")
              << std::endl;
}

TEST_CASE("parse components for HTTP URL", TAG)
{
    auto url = "http://example.com";
    auto actual = getUrlComponents(url);
    auto expected = UrlComponents("http", "example.com", "/", 80);

    REQUIRE(expected == actual);
}

TEST_CASE("parse components for longer HTTP URL", TAG)
{
    auto url = "http://example.com/long/path?query=123&other=890";
    auto actual = getUrlComponents(url);
    auto expected = UrlComponents("http", "example.com", "/long/path?query=123&other=890", 80);

    REQUIRE(expected == actual);
}

TEST_CASE("parse components for HTTPS URL", TAG)
{
    auto url = "https://example.com";
    auto actual = getUrlComponents(url);
    auto expected = UrlComponents("https", "example.com", "/", 443);

    REQUIRE(expected == actual);
}

TEST_CASE("parse components for FILE URL", TAG)
{
    auto url = "file:///Users/joe/temp/file.txt";
    auto actual = getUrlComponents(url);
    auto expected = UrlComponents("file", "", "/Users/joe/temp/file.txt", 80);

    REQUIRE(expected == actual);
}

TEST_CASE("parse components for basic data scheme", TAG)
{
    auto url = "data:,Hello!";
    auto actual = getUrlComponents(url);
    auto expected = UrlComponents("data", "", "", 80, false, "text/plain;charset=US-ASCII", "Hello!");

    REQUIRE(expected == actual);
}

TEST_CASE("parse components for text/html data scheme", TAG)
{
    auto url = "data:text/html,<h1>Hello world!</h1>";
    auto actual = getUrlComponents(url);
    auto expected = UrlComponents("data", "", "", 80, false, "text/html", "<h1>Hello world!</h1>");

    REQUIRE(expected == actual);
}

TEST_CASE("parse components for text/css data scheme", TAG)
{
    auto url = "data:text/css,body{color:red}";
    auto actual = getUrlComponents(url);
    auto expected = UrlComponents("data", "", "", 80, false, "text/css", "body{color:red}");

    REQUIRE(expected == actual);
}

TEST_CASE("parse components for application/json data scheme", TAG)
{
    auto url = "data:application/json,{\"name\":\"John\",\"age\": 25}";
    auto actual = getUrlComponents(url);
    auto expected = UrlComponents("data", "", "", 80, false, "application/json", "{\"name\":\"John\",\"age\": 25}");

    REQUIRE(expected == actual);
}

TEST_CASE("parse components for text/plain;base64 data scheme", TAG)
{
    auto url = "data:text/plain;base64,SGVsbG8h";
    auto actual = getUrlComponents(url);
    auto expected = UrlComponents("data", "", "", 80, true, "text/plain", "SGVsbG8h");

    REQUIRE(expected == actual);
}
