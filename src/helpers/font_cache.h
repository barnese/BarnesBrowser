#pragma once

#include <map>
#include <mutex>
#include <QFont>
#include <QFontDatabase>
#include <QString>
#include <tuple>

/// A thread-safe, function-local singleton font cache.
class FontCache
{
public:
    static FontCache& instance()
    {
        // The cache is only created once when this function is first
        // called. Initialization is guaranteed to be thread-safe.
        static FontCache cache;
        return cache;
    }

    QFont getFont(const QString& family, int size, int weight, int style)
    {
        const Key key{family, size, weight, style};

        std::lock_guard lock(m_mutex);

        if (!m_fonts.contains(key)) 
        { 
            QString fontFamily = family;

            if (family == "monospace")
            {
                QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
                fontFamily = fixedFont.family();
            }
            
            QFont font(fontFamily, size, weight, style); 

            m_fonts[key] = font; 
        } 
        
        return m_fonts[key];   
    }

private:
    // Only allow this class to construct itself, disable the 
    // copy constructor and copy assignment for this singleton.
    FontCache() = default;
    FontCache(const FontCache&) = delete;
    FontCache& operator=(const FontCache&) = delete;

    using Key = std::tuple<QString, int, int, int>;
    std::mutex m_mutex;
    std::map<Key, QFont> m_fonts;
};
