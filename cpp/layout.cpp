#include "layout.h"
#include "constants.h"
#include <QFontMetrics>

Layout::Layout(std::vector<Element> elements)
{
    m_cursorX = 0;
    m_cursorY = 0;
    m_style = QFont::Style::StyleNormal;
    m_weight = QFont::Weight::Normal;

    for (const auto& element : elements) 
    {
        processElement(element);
    }
}

void Layout::processElement(const Element& element) 
{
    if (element.isTag()) 
    {
        std::string tag = element.getText();

        if (tag == "i")
            m_style = QFont::Style::StyleItalic;
        else if (tag == "/i")
            m_style = QFont::Style::StyleNormal;
        else if (tag == "b")
            m_weight = QFont::Weight::Bold;
        else if (tag == "/b")
            m_weight = QFont::Weight::Normal;
    }
    else
    {
        QString content(element.getText().c_str());
        auto words = content.split(' ');

        for (const auto& word : words)
        {
            processWord(word);   
        }
    }
}

void Layout::processWord(const QString& word)
{
   QFont font("Helvetica", 16, m_weight, m_style);
    QFontMetrics metrics(font);
    auto wordWidth = metrics.horizontalAdvance(word);

    if (m_cursorX + wordWidth >= Constants::WIDTH)
    {
        m_cursorY += metrics.lineSpacing() * 1.25;
        m_cursorX = 0;
    }
    Point point(m_cursorX, m_cursorY);
    m_displayList[point] = {word, font};

    m_cursorX += wordWidth + metrics.horizontalAdvance(' '); 
}

DisplayList Layout::getDisplayList() const
{
    return m_displayList;
}
