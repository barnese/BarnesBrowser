#pragma once

#include "commands/command.h"
#include "url.h"

#include <memory>
#include <optional>
#include <vector>

struct CommitData
{
public:
    CommitData(
        URL url, 
        std::optional<int> scroll, 
        int height, 
        std::optional<std::vector<std::unique_ptr<Command>>> displayList
    ) : url(url),
        scroll(scroll),
        height(height),
        displayList(std::move(displayList)) {};

    URL url;
    std::optional<int> scroll;
    int height;
    std::optional<std::vector<std::unique_ptr<Command>>> displayList;
};
