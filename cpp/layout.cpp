#include "layout.h"
#include "constants.h"
#include <QFontMetrics>

Layout::Layout(std::vector<Element> elements)
{
    m_cursorX = 0;
    m_cursorY = 0;
    m_family = "Helvetica";
    m_style = QFont::Style::StyleNormal;
    m_weight = QFont::Weight::Normal;
    m_size = 16;

    for (const auto& element : elements) 
    {
        processElement(element);
    }

    flush();
}

void Layout::processElement(const Element& element) 
{
    if (element.isTag()) 
    {
        std::string tag = element.getText();
        processTag(tag);
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

void Layout::processTag(const std::string& tag)
{
    if (tag == "i")
        m_style = QFont::Style::StyleItalic;
    else if (tag == "/i")
        m_style = QFont::Style::StyleNormal;
    else if (tag == "b")
        m_weight = QFont::Weight::Bold;
    else if (tag == "/b")
        m_weight = QFont::Weight::Normal;
    else if (tag == "small")
        m_size -= 2;
    else if (tag == "/small")
        m_size += 2;
    else if (tag == "big")
        m_size += 4;
    else if (tag == "/big")
        m_size -= 4;
    else if (tag == "br")
        flush();
    else if (tag == "/p")
    {
        flush();
        m_cursorY += Constants::VSTEP;
    }
}

void Layout::processWord(const QString& word)
{
    QFont font = getFont(m_size, m_weight, m_style);
    QFontMetrics metrics(font);
    auto wordWidth = metrics.horizontalAdvance(word);

    if (m_cursorX + wordWidth > Constants::WIDTH)
    {
        flush();
    }

    m_line.push_back({m_cursorX, word, font});

    m_cursorX += wordWidth + metrics.horizontalAdvance(' '); 
}

/// Align the words in a line along the baseline and add them to the display list.
void Layout::flush()
{
    if (m_line.empty())
    {
        return;
    }
   
    auto [maxAscent, maxDescent] = getMaxAscentAndDescentForLine(); 
    int baseline = m_cursorY + 1.25 * maxAscent;

    for (const auto& item : m_line)
    {
        auto [x, word, font] = item;
        QFontMetrics metrics(font);
        int y = baseline - metrics.ascent();
        Point point(x, y);
        m_displayList[point] = {word, font};
    }

    m_cursorY = baseline + 1.25 * maxDescent;
    m_cursorX = 0;
    m_line.clear();
}

std::tuple<int, int> Layout::getMaxAscentAndDescentForLine() const
{
    int maxAscent = 0;
    int maxDescent = 0;

    for (const auto& item : m_line)
    {
        QFont font = std::get<2>(item);
        QFontMetrics metrics(font);
        if (metrics.ascent() > maxAscent)
        {
            maxAscent = metrics.ascent();
        }
        if (metrics.descent() > maxDescent)
        {
            maxDescent = metrics.descent();
        }
    }

    return {maxAscent, maxDescent};
}

DisplayList Layout::getDisplayList() const
{
    return m_displayList;
}

QFont Layout::getFont(int size, int weight, int style) 
{
    std::tuple<int, int, int> key{size, weight, style};
    if (!m_fonts.contains(key))
    {
        QFont font(m_family, size, weight, style);
        m_fonts[key] = font;
    }
    return m_fonts[key];
}