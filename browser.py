import tkinter
from url import URL

# Creates a GUI for the browser using Python's Tk graphical toolkit.
# Requires installing Tkinter: `brew install python-tk`

WIDTH, HEIGHT = 800, 600
HSTEP, VSTEP = 13, 18
SCROLL_STEP = 100
BOTTOM_SCROLL_PADDING = 100

class Browser:
    def __init__(self):
        self.width = WIDTH
        self.height = HEIGHT

        self.window = tkinter.Tk()
        self.canvas = tkinter.Canvas(
            self.window,
            width=self.width,
            height=self.height
        )
        self.canvas.pack(fill=tkinter.BOTH, expand=1)
        self.scroll = 0

        # Bind event handlers
        self.window.bind("<Down>", self.scrolldown)
        self.window.bind("<Up>", self.scrollup)
        self.window.bind("<MouseWheel>", self.scrollmouse)
        self.window.bind("<Button-4>", self.scrollmouse)
        self.window.bind("<Button-5>", self.scrollmouse)
        self.window.bind("<TouchpadScroll>", self.scrolltrackpad)
        self.window.bind("<Configure>", self.resize)

    def load(self, url):
        body = url.request()
        self.text = lex(body)
        self.display_list = layout(self.text, self.width)
        self.draw()

    # Draws the browser's display list of text in screen coordinates.
    def draw(self):
        self.canvas.delete("all")
        for x, y, c in self.display_list:
            # Skip drawing characters that are off-screen.
            if y > self.scroll + self.height: continue
            if y + VSTEP < self.scroll: continue
            self.canvas.create_text(x, y - self.scroll, text=c)

    def resize(self, e):
        self.width = e.width
        self.height = e.height
        self.display_list = layout(self.text, e.width)
        self.draw()

    def scrolldown(self, e):
        if self.allow_scrolldown():
            self.scroll += SCROLL_STEP
            self.draw()
    
    def scrollup(self, e):
        self.scroll -= SCROLL_STEP
        if self.scroll < 0: self.scroll = 0
        self.draw()

    def scrolltrackpad(self, e):
        dx, dy = self.window.tk.call("tk::PreciseScrollDeltas", e.delta)
        dx = float(dx)
        dy = float(dy)

        if dy > 0:
            self.scroll -= SCROLL_STEP
            if self.scroll < 0: self.scroll = 0
        elif dy < 0 and self.allow_scrolldown():
            self.scroll += SCROLL_STEP

        self.draw()

    # Determines if a scrolldown action can be performed, i.e.,
    # the current scroll doesn't exceed the page's contents.
    def allow_scrolldown(self):
        if not self.display_list:
            return False
        
        page_height = self.display_list[-1][1]
        scroll_page_height = page_height

        if page_height > self.height: 
            scroll_page_height -= self.height - BOTTOM_SCROLL_PADDING

        return self.scroll + SCROLL_STEP < scroll_page_height

    def scrollmouse(self, e):
        print(e)
        if e.num == 4:
            delta = 1
        elif e.num == 5:
            delta = -1
        else:
            delta = e.delta
        print(delta)

        if delta > 0:
            self.scroll += SCROLL_STEP
        else:
            self.scroll -= SCROLL_STEP
            if self.scroll < 0: self.scroll = 0
        self.draw()

#END Browser class

# Lays out text using page coordinates.
def layout(text, width):
    display_list = []
    cursor_x, cursor_y = HSTEP, VSTEP

    for c in text:
        display_list.append((cursor_x, cursor_y, c))

        if c == "\n":
            cursor_x = 0
            cursor_y += VSTEP

        cursor_x += HSTEP
        if cursor_x >= width - HSTEP:
            cursor_y += VSTEP
            cursor_x = HSTEP

    return display_list

# Returns the textual context of an HTML document.
def lex(body):
    text = ""
    in_tag = False
    for c in body:
        if c == "<":
            in_tag = True
        elif c == ">":
            in_tag = False
        elif not in_tag:
            text += c
    return text

if __name__ == "__main__":
    import sys
    Browser().load(URL(sys.argv[1]))
    tkinter.mainloop()