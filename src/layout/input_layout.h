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
        Node* node, 
        const Layout* parent, 
        const Layout* previous = nullptr
    );
    void layout() override;
    std::vector<std::unique_ptr<Command>> paint() const override;
    bool shouldPaint() const override;
    std::vector<std::unique_ptr<Command>> paintEffects(std::vector<std::unique_ptr<Command>> commands) const override;

private:
    QFont getFont(QString family, int size, int weight, int style);

    // Font cache indexed by family, size, weight, style.
    std::map<std::tuple<QString, int, int, int>, QFont> m_fonts;
};
