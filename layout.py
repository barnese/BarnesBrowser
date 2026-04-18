import tkinter
from tkinter import font
from lex import Text

LINE_SPACING = 1.25

class Layout:
    def __init__(self, tokens, width, hstep, vstep):
        self.display_list = []
        self.width = width
        self.hstep = hstep
        self.vstep = vstep
        self.cursor_x = hstep
        self.cursor_y = vstep
        self.weight = "normal"
        self.style = "roman"
        self.size = 12

        for tok in tokens:
            self.token(tok)

    def token(self, tok):
        if isinstance(tok, Text):
            for word in tok.text.split():
                self.word(word)
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

    def word(self, word):
        font = tkinter.font.Font(
            size=self.size,
            weight=self.weight,
            slant=self.style
        )
        w = font.measure(word)
        self.display_list.append((self.cursor_x, self.cursor_y, word, font))

        if word == "\n":
            self.cursor_x = 0
            self.cursor_y += font.metrics("linespace") * LINE_SPACING

        self.cursor_x += w + font.measure(" ")
        if self.cursor_x + w >= self.width - self.hstep:
            self.cursor_y += font.metrics("linespace") * LINE_SPACING
            self.cursor_x = self.hstep
