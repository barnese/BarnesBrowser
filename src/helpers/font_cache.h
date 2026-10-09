#pragma once

#include <map>
#include <mutex>
#include <QFont>
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
            QFont font(family, size, weight, style); 

            if (family == "monospace")
                font.setStyleHint(QFont::Monospace);

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
