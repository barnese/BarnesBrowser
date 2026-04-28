import tkinter
from fonts import get_font
from text import Text

HSTEP, VSTEP = 13, 18

# Defines a layout block, such as a paragraph or heading.
class BlockLayout:
    def __init__(self, node, parent, previous, view_source):
        self.node = node
        self.parent = parent
        self.previous = previous
        self.view_source = view_source
        self.children = []

    def layout(self, width):
        self.width = width
        self.display_list = []
        self.cursor_x = HSTEP
        self.cursor_y = VSTEP
        self.weight = "normal"
        self.style = "roman"
        self.size = 12
        
        # Acts as a buffer for correcting baseline positioning of text
        # of different sizes using a two-pass algorithm,
        # first computing x-positions, then y-positions.
        self.line = []

        self.recurse(self.node)
        self.flush()

    def open_tag(self, tag):
        if tag == "i":
            self.syle = "italic"
        elif tag == "b":
            self.weight = "bold"
        elif tag == "small":
            self.size -= 2
        elif tag == "big":
            self.size += 4
        elif tag == "br":
            self.flush()
    
    def close_tag(self, tag):
        if tag == "i":
            self.style = "roman"
        elif tag == "b":
            self.weight = "normal"
        elif tag == "small":
            self.size += 2
        elif tag == "big":
            self.size -= 4
        elif tag == "br":
            self.flush()
        elif tag == "p":
            self.flush()
            # Add a gap between paragraphs.
            self.cursor_y += VSTEP 
 
    # Walks the node tree, handling open and close tags and text.
    def recurse(self, tree):
        if isinstance(tree, Text):
            for word in tree.text.split():
                self.word(word, True)
        else:
            if self.view_source:
                tok_word = "&lt;" + tree.tag + "&gt;"
                self.word(tok_word, False)

            self.open_tag(tree.tag)
            for child in tree.children:
                self.recurse(child)
            self.close_tag(tree.tag)

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
