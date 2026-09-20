#pragma once

#include "layout.h"
#include "html_tree/node.h"

#include <QGraphicsScene>

class DocumentLayoutException : public std::runtime_error
{
public:
    explicit DocumentLayoutException(const std::string& message)
        : std::runtime_error(message) {}
};

/// Represents the layout of the root HTML document.
class DocumentLayout : public Layout
{
public:
    DocumentLayout(Node* node);
    void layout() override;
    void layout(int parentWidth, double zoom);
    std::vector<std::unique_ptr<Command>> paint() const override;
    bool shouldPaint() const override;
    std::vector<std::unique_ptr<Command>> paintEffects(std::vector<std::unique_ptr<Command>> commands) const override;
    std::string describe() const override;

private:
    int m_parentWidth;
};
