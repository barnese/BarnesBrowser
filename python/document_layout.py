from block_layout import BlockLayout

# Defines the root object of the layout tree.
class DocumentLayout:
    def __init__(self, node, view_source):
        self.node = node
        self.view_source = view_source
        self.parent = None
        self.children = []

    def layout(self):
        child = BlockLayout(self.node, self, None, self.view_source)
        self.children.append(child)
        child.layout()
        self.display_list = child.display_list
