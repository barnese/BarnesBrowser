#pragma once

#include <optional>
#include <string>
#include <vector>

/// Splits a string by a delimiter.
std::vector<std::string> split(const std::string& text, const std::string& delimiter, bool firstOnly = true);

/// Splits a string starting at the end.
std::vector<std::string> splitLast(const std::string& text, const std::string& delimiter);

/// Splits a string by spaces, but ignores spaces within quotes.
std::vector<std::string> splitBySpaceQuoted(const std::string& text);

std::string trim(std::string text);
std::string trimLeft(std::string text);
std::string trimRight(std::string text);

std::string toLowercase(std::string text);

std::string removeComments(std::string text);

float percentToFloat(std::string percentString);
float pixelsToFloat(std::string pixelString);
std::optional<int> pixelsToInt(std::string pixelString);
double pixelsToDouble(std::string pixelString);

std::string urlEncode(const std::string& value);
