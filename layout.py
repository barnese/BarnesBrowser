import tkinter
from tkinter import font
from lex import Text

HSTEP, VSTEP = 13, 18

# Global font dictionary for caching fonts to improve performance.
FONTS = {}

def get_font(size, weight, style):
    key = (size, weight, style)
    if key not in FONTS:
        font = tkinter.font.Font(size=size, weight=weight, slant=style)
        label = tkinter.Label(font=font)
        FONTS[key] = (font, label)
    return FONTS[key][0]

class Layout:
    def __init__(self, tokens, width, view_source):
        self.display_list = []
        self.width = width
        self.view_source = view_source
        self.cursor_x = HSTEP
        self.cursor_y = VSTEP
        self.weight = "normal"
        self.style = "roman"
        self.size = 12
        
        # Acts as a buffer for correcting baseline positioning of text
        # of different sizes using a two-pass algorithm,
        # first computing x-positions, then y-positions.
        self.line = []

        for tok in tokens:
            self.token(tok)

        self.flush()

    def token(self, tok):
        if isinstance(tok, Text):
            for word in tok.text.split():
                self.word(word, True)
        elif self.view_source:
            tok_word = "&lt;" + tok.tag + "&gt;"
            self.word(tok_word, False)
        elif tok.tag == "i":
            self.style = "italic"
        elif tok.tag == "/i":
            self.style = "roman"
        elif tok.tag == "b":
            self.weight = "bold"
        elif tok.tag == "/b":
            self.weight = "normal"
        elif tok.tag == "small":
            self.size -= 2
        elif tok.tag == "/small":
            self.size += 2
        elif tok.tag == "big":
            self.size += 4
        elif tok.tag == "/big":
            self.size -= 4
        elif tok.tag == "br":
            self.flush()
        elif tok.tag == "/p":
            self.flush()
            self.cursor_y += VSTEP # Add a gap between paragraphs.
        else:
            self.size = 12
            self.style = "roman"
            self.weight = "normal"

    def word(self, word, add_space):
        word = self.replace_special_chars(word)
        font = get_font(self.size, self.weight, self.style)
        self.line.append((self.cursor_x, word, font))
        w = font.measure(word)
        self.cursor_x += w

        if add_space:
            self.cursor_x += font.measure(" ")

        if self.cursor_x + w > self.width - HSTEP:
            self.flush()

    # Replaces occurrences of special character codes with their matching symbols.
    def replace_special_chars(self, word):
        word = word.replace("&lt;", "<")
        word = word.replace("&gt;", ">")
        return word
   
    # Flush has three resposibilities:
    # 1. Aligns words in a line along the baseline.
    # 2. Adds words to display list.
    # 3. Updates cursor_x and cursor_y fields.
    def flush(self):
        if not self.line: return
        metrics = [font.metrics() for x, word, font in self.line]
        max_ascent = max([metric["ascent"] for metric in metrics])

        baseline = self.cursor_y + 1.25 * max_ascent

        for x, word, font in self.line:
            y = baseline - font.metrics("ascent")
            self.display_list.append((x, y, word, font))
        
        max_descent = max([metric["descent"] for metric in metrics])
        self.cursor_y = baseline + 1.25 * max_descent
        self.cursor_x = HSTEP
        self.line = []
