#pragma once

#include "commands/command.h"
#include "html_tree/node.h"
#include "layout.h"

#include <memory>
#include <QFont>
#include <QString>
#include <vector>

/// Layout object representing an HTML input area.
class InputLayout : public Layout
{
public: 
    InputLayout(
        const Node* node, 
        const Layout* parent, 
        const Layout* previous = nullptr
    );
    void layout() override;
    std::vector<std::unique_ptr<Command>> paint() const override;

private:
    QFont getFont(int size, int weight, int style);

    // Font cache indexed by size, weight, style.
    std::map<std::tuple<int, int, int>, QFont> m_fonts;
};
