#pragma once

#include <string>
#include <vector>

std::vector<std::string> split(std::string text, std::string delimiter, bool firstOnly = true);
std::string trim(std::string text);
std::string trimLeft(std::string text);
std::string trimRight(std::string text);
std::string toLowercase(std::string text);
std::string removeComments(std::string text);
float percentToFloat(std::string percentString);
float pixelsToFloat(std::string pixelString);
