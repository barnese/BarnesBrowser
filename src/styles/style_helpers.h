#pragma once

#include "html_tree/node.h"

#include <string>
#include <QFont>
#include <QString>

QFont::Weight cssFontWeightToQt(const std::string& input);
QFont::Style cssFontStyleToQt(const std::string& style);
void replaceHTMLCharacterEntities(QString& word);
double overrideSizeWithStyle(const Node* node, int size, std::string styleName);

/// Parses font styles on the given node and returns a font.
QFont getFont(const Node* node, double zoom, int adjustSize = 0);
