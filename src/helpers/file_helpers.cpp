#include "file_helpers.h"

#include <filesystem>
#include <fstream>

std::vector<std::byte> readFile(const std::string& path)
{
    std::vector<std::byte> data(std::filesystem::file_size(path));

    std::ifstream file(path, std::ios::binary);

    file.read(
        reinterpret_cast<char*>(data.data()),
        static_cast<std::streamsize>(data.size())
    );

    return data;
}

std::string getNewTabHtmlFilePath()
{
    std::filesystem::path path = "assets/pages/new_tab.html";
    auto newTabHtmlPath = std::filesystem::absolute(path);
    return "file://" + newTabHtmlPath.string();
}
