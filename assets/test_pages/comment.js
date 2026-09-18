window.console.log("Hello from comment.js!");

var strong = window.document.querySelectorAll("strong")[0];
var allowSubmit = true;

function lengthCheck() {
    var value = this.getAttribute("value");
    allowSubmit = value.length <= 100;
    if (!allowSubmit) {
        strong.innerHTML = "Comment too long!";
    }
}

var inputs = window.document.querySelectorAll("input");
for (var i = 0; i < inputs.length; i++) {
    inputs[i].addEventListener("keydown", lengthCheck);
}

var forms = window.document.querySelectorAll("form");
if (forms.length > 0) {
    var form = forms[0];
    form.addEventListener("submit", function(e) {
        if (!allowSubmit) e.preventDefault();
    });
}
