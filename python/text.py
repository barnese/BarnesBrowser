# Represents a text node at the leaf of the HTML tree (DOM).
class Text:
    def __init__(self, text, parent):
        self.text = text
        self.children = []
        self.parent = parent

    # Gives a pretty printed version of this object.
    def __repr__(self):
        return repr(self.text)
    