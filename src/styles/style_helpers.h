#pragma once

#include <QFont>
#include <QString>

QFont::Weight cssFontWeightToQt(const std::string& input);
QFont::Style cssFontStyleToQt(const std::string& style);
void replaceHTMLCharacterEntities(QString& word);
