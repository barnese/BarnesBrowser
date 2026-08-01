// A runtime for wrapping C++ functions to be called in JavaScript.

console = { log: function(x) { 
    printMessage(x); 
}}

// Defines a Node object that wraps a node handle.
function Node(handle) { this.handle = handle; }

Node.prototype.getAttribute = function(attr) {
    return currentJSContext.getAttribute(this.handle, attr);
}

listeners = {};

Node.prototype.addEventListener = function(type, listener) {
    if (!listeners[this.handle]) listeners[this.handle] = {};
    var dict = listeners[this.handle];
    if (!dict[type]) dict[type] = [];
    var list = dict[type];
    list.push(listener);
}

Node.prototype.dispatchEvent = function(type) {
    var handle = this.handle;
    var list = (listeners[handle] && listeners[handle][type]) || [];
    for (var i = 0; i < list.length; i++) {
        list[i].call(this);
    }
}

document = { querySelectorAll: function(s) {
    var handles = currentJSContext.querySelectorAll(s);
    return handles.map(function(h) { return new Node(h) });
}}
