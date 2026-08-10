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

TEST_CASE("splitLast() splits a string at the last delimiter", TAG)
{
    std::vector<std::string> expected { "a", "b" };
    auto actual = splitLast("a/b", "/");
    REQUIRE(expected == actual);
}

TEST_CASE("splitLast() splits a string at the last delimiter with multiple delimiters", TAG)
{
    std::vector<std::string> expected { "a/b", "c" };
    auto actual = splitLast("a/b/c", "/");
    REQUIRE(expected == actual);
}

TEST_CASE("splitLast() splits a string at the last delimiter with multiple delimiters at start and end", TAG)
{
    std::vector<std::string> expected { "/a/b/c", "" };
    auto actual = splitLast("/a/b/c/", "/");
    REQUIRE(expected == actual);
}

TEST_CASE("splitLast() doesn't split a string with no delimiter", TAG)
{
    std::vector<std::string> expected { "asdf" };
    auto actual = splitLast("asdf", "/");
    REQUIRE(expected == actual);
}

TEST_CASE("splitBySpaceQuoted() empty string", TAG)
{
    std::vector<std::string> expected { };
    auto actual = splitBySpaceQuoted("");
    REQUIRE(expected == actual);
}

TEST_CASE("splitBySpaceQuoted() string without quotes", TAG)
{
    std::vector<std::string> expected { "head" };
    auto actual = splitBySpaceQuoted("head");
    REQUIRE(expected == actual);
}

TEST_CASE("splitBySpaceQuoted() multi-part string without quotes", TAG)
{
    std::vector<std::string> expected { "!DOCTYPE", "html" };
    auto actual = splitBySpaceQuoted("!DOCTYPE html");
    REQUIRE(expected == actual);
}

TEST_CASE("splitBySpaceQuoted() string with quotes with spaces", TAG)
{
    std::vector<std::string> expected { "div", "style=\"color: blue; font-size: 12px;\"" };
    auto actual = splitBySpaceQuoted("div style=\"color: blue; font-size: 12px;\"");
    REQUIRE(expected == actual);
}

TEST_CASE("splitBySpaceQuoted() string with quotes without spaces", TAG)
{
    std::vector<std::string> expected { "html", "lang=\"en-US\"", "xml:lang=\"en-US\"" };
    auto actual = splitBySpaceQuoted("html lang=\"en-US\" xml:lang=\"en-US\"");
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

TEST_CASE("remove comments after code", TAG)
{
    REQUIRE(removeComments("int a = 0; /* comment after */") == "int a = 0; ");
}

TEST_CASE("remove comments before code", TAG)
{
    REQUIRE(removeComments("/* comment */int a = 0;") == "int a = 0;");
}

TEST_CASE("remove comments inside code", TAG)
{
    REQUIRE(removeComments("int a = 0/* comment */;") == "int a = 0;");
}

TEST_CASE("remove multiline comments", TAG)
{
    REQUIRE(removeComments("/* line one\nline two\nline three */\nint a = 0;") == "\nint a = 0;");
}

TEST_CASE("urlEncode() encodes various strings", TAG)
{
    REQUIRE(urlEncode("") == "");
    REQUIRE(urlEncode("abcdefghijklmnopqrstuvwxyz") == "abcdefghijklmnopqrstuvwxyz");
    REQUIRE(urlEncode("-1234567890~") == "-1234567890~");
    REQUIRE(urlEncode("C++ & C#") == "C%2B%2B%20%26%20C%23");
    REQUIRE(urlEncode("café") == "caf%C3%A9");
    REQUIRE(urlEncode("[]{}()") == "%5B%5D%7B%7D%28%29");
    REQUIRE(urlEncode("\"`'") == "%22%60%27");
    REQUIRE(urlEncode("<=>") == "%3C%3D%3E");
    REQUIRE(urlEncode("|@") == "%7C%40");
}
