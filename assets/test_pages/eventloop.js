console.log("[eventloop.js] Hello!");

// Simple counter using requestAnimationFrame 
var count = 0;
function callback() {
    // Simulate slowness
    //for (var i = 0; i < 5e6; i++);
    var output = window.document.querySelectorAll("div")[1];
    output.innerHTML = "count: " + (count++);
    if (count < 100) {
        requestAnimationFrame(callback);
    }
}
requestAnimationFrame(callback);
