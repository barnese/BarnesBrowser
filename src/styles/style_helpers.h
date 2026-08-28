#pragma once

#include "html_tree/node.h"

#include <string>
#include <QFont>
#include <QString>

QFont::Weight cssFontWeightToQt(const std::string& input);
QFont::Style cssFontStyleToQt(const std::string& style);
void replaceHTMLCharacterEntities(QString& word);
int overrideSizeWithStyle(const Node* node, int size, std::string styleName);
