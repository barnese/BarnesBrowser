// Uncomment this to test setTimeout.
//function callback() {
//    console.log("Hello from tests.js!");
//}
//setTimeout(callback, 10);

// Uncomment this to test XMLHttpRequest.
//xhr = new XMLHttpRequest();
//xhr.open("GET", "http://localhost:8000/fonts.html", true);
//xhr.onload = function() {
//    console.log("[test.js] received response: " + xhr.responseText.slice(0, 20) + "...");
//};
//xhr.onerror = function() {
//    console.log("[test.js] request failed");
//};
//xhr.send();

function lengthCheck() {
    var name = this.getAttribute("name");
    var value = this.getAttribute("value");
    if (value.length > 10) {
        console.log("Input " + name + " has too much text.");
    }
}

var inputs = document.querySelectorAll('input');
for (var i = 0; i < inputs.length; i++) {
    inputs[i].addEventListener("keydown", lengthCheck);
}
