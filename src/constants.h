#pragma once

#include <map>
#include <string>
#include <vector>

namespace Constants
{
    const int HSTEP = 13;
    const int VSTEP = 18;
    const std::string HTML_HEADER_SEPARATOR = "\r\n\r\n";
    const int SCROLL_STEP = 100;
    const int INPUT_WIDTH_PX = 200;
    
    /// Target refresh rate, 33 frames per millisecond.
    const int REFRESH_RATE_MS = 33;

    const std::vector<std::string> BLOCK_ELEMENTS = 
    {
        "html", "body", "article", "section", "nav", "aside",
        "h1", "h2", "h3", "h4", "h5", "h6", "hgroup", "header",
        "footer", "address", "p", "hr", "pre", "blockquote",
        "ol", "ul", "menu", "li", "dl", "dt", "dd", "figure",
        "figcaption", "main", "div", "table", "form", "fieldset",
        "legend", "details", "summary"
    };

    // Defaults for inherited CSS properties.
    const std::map<std::string, std::string> INHERITED_PROPERTIES = 
    {
        { "font-size", "16px" },
        { "font-style", "normal" },
        { "font-weight", "normal" },
        { "color", "black" },
    };

    const std::map<std::string, std::string> HTML_CHARACTER_ENTITIES = 
    {
        { "&nbsp;", " " },
        { "&lt;", "<" },
        { "&gt;", ">" },
        { "&amp;", "&" },
        { "&quot;", "\"" },
        { "&apos;", "'" },
        { "&cent;", "¢" },
        { "&pound;", "£" },
        { "&yen;", "¥" },
        { "&euro;", "€" },
        { "&copy;", "©" },
        { "&reg;", "®" },
        { "&trade;", "™" },
        { "&ndash;", "–" },
        { "&mdash;", "—" },
        { "&sect;", "§" }, 
        { "&deg;", "°" }, 
        { "&para;", "¶" },
        { "&times;", "×" }, 
        { "&divide;", "÷" }, 
        { "&plusmn;", "±" }, 
        { "&infin;", "∞" }, 
        { "&ne;", "≠" }, 
        { "&le;", "≤" }, 
        { "&ge;", "≥" },
        { "&hellip;", "…" }, 
        { "&bull;", "•" }, 
        { "&ldquo;", "“" },
        { "&rdquo;", "”" }, 
        { "&lsquo;", "‘" }, 
        { "&rsquo;", "’" },
        { "&larr;", "←" }, 
        { "&rarr;", "→" }, 
        { "&uarr;", "↑" }, 
        { "&darr;", "↓" }
    };
}
