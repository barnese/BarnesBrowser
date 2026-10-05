var div = window.document.querySelectorAll("div")[0];

function start_fade_out(e) {
    window.console.log("[opacity-transition.js] start fade out");
    div.style = "opacity:0.1";
    e.preventDefault();
}

function start_fade_in(e) {
    window.console.log("[opacity-transition.js] start fade in");
    div.style = "opacity:0.999";
    e.preventDefault();
}

var buttons = window.document.querySelectorAll("button");
buttons[0].addEventListener("click", start_fade_out);
buttons[1].addEventListener("click", start_fade_in);
