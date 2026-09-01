console.log("[opacity.js] hello!");

// TEST 1: Uncomment this to test setTimeout.
//function callback() {
//    console.log("Hello from tests.js!");
//}
//setTimeout(callback, 10);


// TEST 2: Uncomment this to test XMLHttpRequest.
//xhr = new XMLHttpRequest();
//xhr.open("GET", "http://localhost:8000/fonts.html", true);
//xhr.onload = function() {
//    console.log("[test.js] received response: " + xhr.responseText.slice(0, 20) + "...");
//};
//xhr.onerror = function() {
//    console.log("[test.js] request failed");
//};
//xhr.send();

// TEST 3: Opacity animation test
// var div = document.querySelectorAll("div")[1];
// var totalFrames = 120;
// var currentFrame = 0;
// var changePerFrame = (0.999 - 0.1) / totalFrames;
// function animate() {
//     currentFrame++;
//     var newOpacity = currentFrame * changePerFrame + 0.1;
//     div.style = "opacity:" + newOpacity;
//     return currentFrame < totalFrames;
// }

// function runAnimationFrame() {
//     if (animate())
//         requestAnimationFrame(runAnimationFrame);
// }
//requestAnimationFrame(runAnimationFrame);
// END TEST 3


// Other functions

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
