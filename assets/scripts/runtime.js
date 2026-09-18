// A runtime for implementing the JavaScript API 
// to work with our browser.

window.EVENT_LISTENERS = {};
window.SET_TIMEOUT_REQUESTS = {};
window.XHR_REQUESTS = {};
window.RAF_LISTENERS = [];
window.WINDOW_LISTENERS = {};

window.console = { log: function(x) { 
    printMessage(x); 
}}

window.document = { querySelectorAll: function(s) {
    var handles = currentJSContext.querySelectorAll(s, window._id);
    return handles.map(function(h) { return new window.Node(h) });
}}

window.Event = function(type) {
    this.type = type;
    this.do_default = true;
}

window.Event.prototype.preventDefault = function() {
    this.do_default = false;
}

// Defines a Node object that wraps a node handle.
window.Node = function(handle) { 
    this.handle = handle; 
}

window.Node.prototype.getAttribute = function(attr) {
    return currentJSContext.getAttribute(this.handle, attr, window._id);
}

window.Node.prototype.setAttribute = function(attr, value) {
    currentJSContext.setAttribute(this.handle, attr, value, window._id);
}

window.Node.prototype.addEventListener = function(type, listener) {
    if (!window.EVENT_LISTENERS[this.handle]) 
        window.EVENT_LISTENERS[this.handle] = {};
    var dict = window.EVENT_LISTENERS[this.handle];
    if (!dict[type]) dict[type] = [];
    var list = dict[type];
    list.push(listener);
}

window.Node.prototype.dispatchEvent = function(event) {
    var type = event.type;
    var handle = this.handle;
    window.console.log("[runtime.js] dispatching event " + type + ", handle: " + handle);
    var list = (window.EVENT_LISTENERS[handle] && 
        window.EVENT_LISTENERS[handle][type]) || [];
    for (var i = 0; i < list.length; i++) {
        list[i].call(this, event);
    }
    return event.do_default;
}

Object.defineProperty(window.Node.prototype, 'innerHTML', {
    set: function(s) {
        currentJSContext.setInnerHtml(this.handle, s.toString(), window._id);
    }
});

Object.defineProperty(window.Node.prototype, 'style', {
    set: function(s) {
        currentJSContext.setStyle(this.handle, s.toString(), window._id);
    }
});

window.XMLHttpRequest = function() {
    this.handle = Object.keys(window.XHR_REQUESTS).length;
    window.XHR_REQUESTS[this.handle] = this;
}

window.XMLHttpRequest.prototype.open = function(method, url, isAsync) {
    this.isAsync = isAsync;
    this.method = method;
    this.url = url;
}

window.XMLHttpRequest.prototype.send = function(body) {
    if (body === undefined) {
        body = "";
    }
    this.responseText = currentJSContext.sendXmlHttpRequest(
        this.handle, this.method, this.url, body, this.isAsync, window._id
    );
}

window.__runXhrOnload = function(handle, body) {
    var obj = window.XHR_REQUESTS[handle];
    var evt = new window.Event('load');
    obj.responseText = body;
    if (obj.onload)
        obj.onload(evt);
}

window.setTimeout = function(callback, timeDelta) {
    var handle = Object.keys(window.SET_TIMEOUT_REQUESTS).length;
    window.SET_TIMEOUT_REQUESTS[handle] = callback;
    currentJSContext.setTimeout(handle, timeDelta, window._id);
}

window.__runSetTimeout = function(handle) {
    var callback = window.SET_TIMEOUT_REQUESTS[handle];
    callback();
}

window.requestAnimationFrame = function(fn) {
    window.RAF_LISTENERS.push(fn);
    currentJSContext.requestAnimationFrame();
}

window.__runRAFHandlers = function() {
    var handlers_copy = window.RAF_LISTENERS;
    window.RAF_LISTENERS = [];
    for (var i = 0; i < handlers_copy.length; i++) {
        handlers_copy[i]();
    }
}

// To allow cross-origin iframes to communicate, the following 
// implements message passing.

window.MessageEvent = function(data) {
    this.type = "message";
    this.data = data;
}

Window.prototype.postMessage = function(message, origin) {
    currentJSContext.postMessage(this._id, message, origin);
}

Window.prototype.addEventListener = function(type, listener) {
    if (!window.WINDOW_LISTENERS[this.handle])
        window.WINDOW_LISTENERS[this.handle] = {};
    var dict = window.WINDOW_LISTENERS[this.handle];
    if (!dict[type]) dict[type] = [];
    var list = dict[type];
    list.push(listener);
}

Window.prototype.dispatchEvent = function(evt) {
    var type = evt.type;
    var handle = this.handle
    var list = (window.WINDOW_LISTENERS[handle] &&
        window.WINDOW_LISTENERS[handle][type]) || [];
    for (var i = 0; i < list.length; i++) {
        list[i].call(this, evt);
    }

    return evt.do_default;
}

// window.parent can look up the correct window in the global windows map.
Object.defineProperty(Window.prototype, 'parent', {
  configurable: true,
  get: function() {
    var parent_id = call_python('parent', window._id);
    if (parent_id != undefined) {
        var parent = WINDOWS[parent_id];
        if (parent === undefined) parent = new Window(parent_id);
        return parent;
    }
  }
});
