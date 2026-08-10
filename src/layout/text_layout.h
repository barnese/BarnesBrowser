#pragma once

#include "commands/command.h"
#include "html_tree/node.h"
#include "layout.h"

#include <memory>
#include <QFont>
#include <QString>
#include <vector>

class TextLayout : public Layout
{
public: 
    TextLayout(
        Node* node, 
        QString word, 
        const Layout* parent, 
        const Layout* previous = nullptr
    );
    void layout() override;
    std::vector<std::unique_ptr<Command>> paint() const override;
    bool shouldPaint() const override;

private:
    QFont getFont(QString family, int size, int weight, int style);

    QString word;

    // Font cache indexed by family, size, weight, style.
    std::map<std::tuple<QString, int, int, int>, QFont> m_fonts;
};
