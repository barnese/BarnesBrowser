#pragma once

#include "html_tree/node.h"
#include "layout/protected_field.h"

#include <string>
#include <QFont>
#include <QString>

QFont::Weight cssFontWeightToQt(const std::string& input);
QFont::Style cssFontStyleToQt(const std::string& style);
double overrideSizeWithStyle(const Node* node, int size, std::string styleName);

/// Parses font styles on the given node and returns a font.
QFont getFont(Node* node, double zoom, int adjustSize, Invalidatable* notify);
