// A runtime for implementing the JavaScript API 
// to work with our browser.

EVENT_LISTENERS = {};
SET_TIMEOUT_REQUESTS = {};
XHR_REQUESTS = {};
RAF_LISTENERS = [];

console = { log: function(x) { 
    printMessage(x); 
}}

function Event(type) {
    this.type = type;
    this.do_default = true;
}

Event.prototype.preventDefault = function() {
    this.do_default = false;
}

// Defines a Node object that wraps a node handle.
function Node(handle) { 
    this.handle = handle; 
}

Node.prototype.getAttribute = function(attr) {
    return currentJSContext.getAttribute(this.handle, attr);
}

Node.prototype.addEventListener = function(type, listener) {
    if (!EVENT_LISTENERS[this.handle]) EVENT_LISTENERS[this.handle] = {};
    var dict = EVENT_LISTENERS[this.handle];
    if (!dict[type]) dict[type] = [];
    var list = dict[type];
    list.push(listener);
}

Node.prototype.dispatchEvent = function(event) {
    var type = event.type;
    var handle = this.handle;
    var list = (EVENT_LISTENERS[handle] && EVENT_LISTENERS[handle][type]) || [];
    for (var i = 0; i < list.length; i++) {
        list[i].call(this);
    }
    return event.do_default;
}

document = { querySelectorAll: function(s) {
    var handles = currentJSContext.querySelectorAll(s);
    return handles.map(function(h) { return new Node(h) });
}}

Object.defineProperty(Node.prototype, 'innerHTML', {
    set: function(s) {
        currentJSContext.setInnerHtml(this.handle, s.toString());
    }
});

function XMLHttpRequest() {
    this.handle = Object.keys(XHR_REQUESTS).length;
    XHR_REQUESTS[this.handle] = this;
}

XMLHttpRequest.prototype.open = function(method, url, isAsync) {
    this.isAsync = isAsync;
    this.method = method;
    this.url = url;
}

XMLHttpRequest.prototype.send = function(body) {
    if (body === undefined) {
        body = "";
    }
    console.log("[runtime.js] Sending XMLHttpRequest...");
    this.responseText = currentJSContext.sendXmlHttpRequest(
        this.handle, this.method, this.url, body, this.isAsync
    );
}

function __runXhrOnload(handle, body) {
    console.log("[runtime.js] __runXhrOnload called.");
    var obj = XHR_REQUESTS[handle];
    var evt = new Event('load');
    obj.responseText = body;
    if (obj.onload)
        obj.onload(evt);
}

function setTimeout(callback, timeDelta) {
    var handle = Object.keys(SET_TIMEOUT_REQUESTS).length;
    SET_TIMEOUT_REQUESTS[handle] = callback;
    currentJSContext.setTimeout(handle, timeDelta);
}

function __runSetTimeout(handle) {
    var callback = SET_TIMEOUT_REQUESTS[handle];
    callback();
}

function requestAnimationFrame(fn) {
    RAF_LISTENERS.push(fn);
    currentJSContext.requestAnimationFrame();
}

function __runRAFHandlers() {
    var handlers_copy = RAF_LISTENERS;
    RAF_LISTENERS = [];
    for (var i = 0; i < handlers_copy.length; i++) {
        handlers_copy[i]();
    }
}
