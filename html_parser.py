from text import Text
from element import Element

class HTMLParser:
    SELF_CLOSING_TAGS = [
        "area", "base", "br", "col", "embed", "hr", "img", "input",
        "link", "meta", "param", "source", "track", "wbr"
    ]

    def __init__(self, body):
        self.body = body

        # Stores unfinished nodes as the parser reads the HTML body.
        self.unfinished = []

    def parse(self):
        text = ""
        in_tag = False
        for c in self.body:
            if c == "<":
                in_tag = True
                if text: self.add_text(text)
                text = ""
            elif c == ">":
                in_tag = False
                self.add_tag(text)
                text = ""
            else:
                text += c
        if not in_tag and text:
            self.add_text(text)
        return self.finish()

    # Adds a text node as a child of the last unfinished node.
    def add_text(self, text):
        # Skip whitesapce-only text for now. Revisit to render <span> correctly.
        if text.isspace(): return
        parent = self.unfinished[-1]
        node = Text(text, parent)
        parent.children.append(node)

    # Adds tags to the tree.
    def add_tag(self, tag):
        tag, attributes = self.get_attributes(tag)

        # Throw away tags like <!doctype> or comments <!-- comment -->.
        if tag.startswith("!"): return

        if tag.startswith("/"):
            # If this is the last close tag, there will be no 
            # unfinished node to add it to, so just return.
            if len(self.unfinished) == 1: return

            # Otherwise, this close tag finishes the last unfinished node
            # by adding it to the previous unfinished node in the list.
            node = self.unfinished.pop()
            parent = self.unfinished[-1]
            parent.children.append(node)
        elif tag in self.SELF_CLOSING_TAGS:
            # This is a self-closing tag, so we need to auto-close it.
            parent = self.unfinished[-1]
            node = Element(tag, attributes, parent)
            parent.children.append(node)
        else:
            # Add this open tag to the end of the unfinished node list.
            # Note: if this is the very first open tag, it will have no parent.
            parent = self.unfinished[-1] if self.unfinished else None
            node = Element(tag, attributes, parent)
            self.unfinished.append(node)
    
    # Once the parser is done, it turns the incomplete HTML tree into a complete tree
    # by finishing any unfinished nodes.
    def finish(self):
        while len(self.unfinished) > 1:
            node = self.unfinished.pop()
            parent = self.unfinished[-1]
            parent.children.append(node)
        return self.unfinished.pop()

    def get_attributes(self, text):
        parts = text.split()
        # HTML tag names are case insensitive, so lower-case them.
        tag = parts[0].casefold() 
        attributes = {}
        for attrpair in parts[1:]:
            if "=" in attrpair:
                key, value = attrpair.split("=", 1)
                # The value can be quoted, so the quotes need to be stripped out.
                if len(value) > 2 and value[0] in ["'", "\""]:
                    value = value[1:-1]
                attributes[key.casefold()] = value
            else:
                attributes[attrpair.casefold()] = ""
        return tag, attributes

# End of HTMLParser

def print_tree(node, indent=0):
    print(" " * indent, node)
    for child in node.children:
        print_tree(child, indent + 2)