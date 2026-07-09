#pragma once

#include <string>
#include <vector>

std::vector<std::string> split(const std::string& str, const std::string& delimiter, bool firstOnly = true);
std::string trim(const std::string& str);
std::string trimLeft(const std::string& str);
std::string trimRight(const std::string& str);