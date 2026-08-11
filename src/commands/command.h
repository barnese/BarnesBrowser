#pragma once

#include <string>
#include <QImage>
#include <QRect>

/// Defines an interface for a graphics command for a browser scene.
class Command
{
public:
    virtual ~Command() = default;
    virtual void execute(QImage* surface) = 0;
    virtual std::string describe() const = 0;

    QRect rect;
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
