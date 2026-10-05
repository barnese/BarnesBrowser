#pragma once

#include "accessibility/accessibility_node.h"
#include "commands/command.h"
#include "commands/visual_effect.h"
#include "html_tree/node.h"
#include "html_tree/element.h"
#include "networking/url.h"

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
        bool rootFrameFocused,
        int height, 
        std::optional<std::vector<std::unique_ptr<Command>>> displayList,
        std::map<Node*, VisualEffect*> compositedUpdates,
        std::optional<std::unique_ptr<AccessibilityNode>> accessibilityTree,
        Element* focus
    ) : url(url),
        scroll(scroll),
        rootFrameFocused(rootFrameFocused),
        height(height),
        displayList(std::move(displayList)),
        compositedUpdates(compositedUpdates),
        accessibilityTree(std::move(accessibilityTree)),
        focus(focus)
    {};

    URL url;
    std::optional<int> scroll;
    bool rootFrameFocused;
    int height;
    std::optional<std::vector<std::unique_ptr<Command>>> displayList;
    std::map<Node*, VisualEffect*> compositedUpdates;
    std::optional<std::unique_ptr<AccessibilityNode>> accessibilityTree;
    Element* focus;
};
