# Represents a tag node in the HTML tree.
class Element:
    def __init__(self, tag, attributes, parent):
        self.tag = tag
        self.attributes = attributes
        self.children = []
        self.parent = parent
    
    # Gives a pretty printed version of this object.
    def __repr__(self):
        return "<" + self.tag + ">"