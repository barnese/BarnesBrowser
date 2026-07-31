// A runtime for wrapping C++ functions to be called in JavaScript.

console = { log: function(x) { 
    printMessage(x); 
}}

// Defines a Node object that wraps a node handle.
function Node(handle) { this.handle = handle; }

Node.prototype.getAttribute = function(attr) {
    return currentJSContext.getAttribute(this.handle, attr);
}

document = { querySelectorAll: function(s) {
    var handles = currentJSContext.querySelectorAll(s);
    return handles.map(function(h) { return new Node(h) });
}}
