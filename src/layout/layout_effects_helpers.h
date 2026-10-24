#pragma once

#include "commands/command.h"
#include "html_tree/node.h"

#include <memory>
#include <string>
#include <vector>
#include <QRect>

/// @brief Helper function to paint visual effects.
/// @param node the HTML tree node
/// @param commands a list of layout commands to apply the effects to
/// @param rect the bounding box
/// @returns a new set of commands with the effects applied
std::vector<std::unique_ptr<Command>> paintVisualEffects(
    Node* node, 
    std::vector<std::unique_ptr<Command>> commands, 
    QRectF rect
);

struct ParseOutlineResult
{
    int thickness;
    std::string color;
};

/// Parses a CSS outline property.
std::optional<ParseOutlineResult> parseOutline(const std::string& outlineString);

/// Adds a DrawOwnline command to the given commands.
void paintOutline(Node* node, std::vector<std::unique_ptr<Command>>& commands, QRectF rect, double zoom);
