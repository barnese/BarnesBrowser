console.log("Hello from comment.js!");

var strong = document.querySelectorAll("strong")[0];
var allowSubmit = true;

function lengthCheck() {
    var value = this.getAttribute("value");
    allowSubmit = value.length <= 10;
    if (!allowSubmit) {
        strong.innerHTML = "Comment too long!";
    }
}

var inputs = document.querySelectorAll("input");
for (var i = 0; i < inputs.length; i++) {
    inputs[i].addEventListener("keydown", lengthCheck);
}

var form = document.querySelectorAll("form")[0];
form.addEventListener("submit", function(e) {
    if (!allowSubmit) e.preventDefault();
});
