console.log("Hi from JS! Checking input lengths...");

inputs = document.querySelectorAll('input');
for (var i = 0; i < inputs.length; i++) {
    var name = inputs[i].getAttribute("name");
    var value = inputs[i].getAttribute("value");
    if (value.length > 10) {
        console.log("Input " + name + " has too much text.");
    } else {
        console.log("Input " + name + " is fine.");
    }
}
