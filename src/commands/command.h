#pragma once

#include "paint_context.h"
#include "html_tree/node.h"

#include <string>
#include <QImage>
#include <QRect>

/// Defines an interface for a painting and visual effects commands
/// used to draw the visual content of a web page. Commands form
/// trees, hence the ability for a command to have children and parent.
class Command
{
public:
    virtual ~Command() = default;

    // Executes this command.
    virtual void execute(PaintContext& context) = 0;

    // Provide the ability to clone this command, but with new children.
    virtual std::unique_ptr<Command> clone(std::unique_ptr<Command> child) = 0;

    // Return a string describing this command.
    virtual std::string describe() const = 0;

    QRect rect;
    Command* parent;
    std::vector<std::unique_ptr<Command>> children;

    // For keeping track of the HTML tree node that
    // painted this display item as a sort of identifier.
    // Can be used for compositing animations.
    Node* node;

};

/// @brief Helper that takes any number of unique pointers and puts them
/// into a vector, handling the appropriate ownership moves.
/// @returns a vector of commands
template <typename... Ts>
std::vector<std::unique_ptr<Command>> commandList(std::unique_ptr<Ts>... cmds)
{
    std::vector<std::unique_ptr<Command>> result;
    result.reserve(sizeof...(Ts));
    // Fold expression that moves each pointer into the result vector.
    (result.push_back(std::move(cmds)), ...);
    return result;
}
