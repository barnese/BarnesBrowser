#pragma once

#include "commands/command.h"
#include "commands/visual_effect.h"
#include "html_tree/node.h"
#include "url.h"

#include <map>
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
        std::optional<std::vector<std::unique_ptr<Command>>> displayList,
        std::map<Node*, VisualEffect*> compositedUpdates
    ) : url(url),
        scroll(scroll),
        height(height),
        displayList(std::move(displayList)),
        compositedUpdates(compositedUpdates) {};

    URL url;
    std::optional<int> scroll;
    int height;
    std::optional<std::vector<std::unique_ptr<Command>>> displayList;
    std::map<Node*, VisualEffect*> compositedUpdates;
};
