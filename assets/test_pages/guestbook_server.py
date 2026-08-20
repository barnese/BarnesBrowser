import html
import random
import socket
import urllib.parse

# This is a simple test server that implements
# an anonymous guest book.

# Stores text entries in the guest book.
ENTRIES = [ 
    ("Hello from the United States!", "user1"),
    ("¡Hola desde Argentina!", "user2"),
]

# Maps user tokens to session data dictionaries. 
SESSIONS = {}

# Hard-coded user/password pairs.
LOGINS = {
    "user1": "password1",
    "user2": "password2",
    "u1": "a",
}

# Handles the connection by parsing the request,
# performing the request, then returning a response.
def handle_connection(conx):
    req = conx.makefile("b")
    reqline = req.readline().decode('utf8')
    method, url, version = reqline.split(" ", 2)
    assert method in ["GET", "POST"]
    print("Received connection:", method, url)
    headers = {}
    while True:
        line = req.readline().decode('utf8')
        if line == '\r\n': break
        header, value = line.split(":", 1)
        headers[header.casefold()] = value.strip()

    if 'content-length' in headers:
        length = int(headers['content-length'])
        body = req.read(length).decode('utf-8')
    else:
        body = None

    # Identify users by a token cookie. Create one if not found.
    if "cookie" in headers:
        token = headers["cookie"][len("token="):]
    else:
        token = str(random.random())[2:]

    session = SESSIONS.setdefault(token, {})
    status, body = do_request(session, method, url, headers, body)

    response = "HTTP/1.0 {}\r\n".format(status)
    response += "Content-Length: {}\r\n".format(len(body.encode("utf8")))
    if "cookie" not in headers:
        template = "Set-Cookie: token={}; SameSite=Lax\r\n"
        response += template.format(token)
    csp = "default-src http://localhost:8000"
    response += "Content-Security-Policy: {}\r\n".format(csp)
    response += "\r\n" + body
    conx.send(response.encode('utf8'))
    conx.close()

def show_comments(session):
    out = "<!doctype html>"
    out += "<link rel=\"stylesheet\" href=\"comment.css\" />"
    out += "<script src=\"/comment.js\"></script>"

    if "user" in session:
        # To help prevent cross-site request forgery, create a nonce
        # that is part of the add form and saved to the user session.
        nonce = str(random.random())[2:]
        session["nonce"] = nonce

        out += "<h1>Hello, " + session["user"] + "! Welcome to the Guest Book!</h1>"
        out += "<form action=add method=post>"
        out += "  <p><input name=guest></p>"
        out += "  <input name=nonce type=hidden value=" + nonce + ">"
        out += "  <p><button>Sign the book!</button></p>"
        out += "</form>"
    else:
        out += "<a href=/login>Sign in to write in the guest book</a>"

    # Add an empty strong that can be used to display 
    # client-side validation error messages.
    out += "<strong></strong>"

    for entry, who in ENTRIES:
        out += "<p>" + html.escape(entry) + "\n" 
        out += "<i>by " + html.escape(who) + "</i></p>"

    # Test Content-Security-Policy by adding a bad script.
    out += "<script src=https://example.com/evil.js></script>"

    return out

def form_decode(body):
    params = {}
    if body is None or len(body) == 0:
        return params
    for field in body.split("&"):
        name, value = field.split("=", 1)
        name = urllib.parse.unquote_plus(name)
        value = urllib.parse.unquote_plus(value)
        params[name] = value
    return params

def add_entry(session, params):
    if "user" not in session: return

    # To prevent CSRF, make sure the user's nonce matches the
    # one we generated when displaying the add form.
    if "nonce" not in session or "nonce" not in params: return
    if session["nonce"] != params["nonce"]: return

    if 'guest' in params and len(params['guest']) <= 100:
        ENTRIES.append((params['guest'], session["user"]))

def login_form(session):
    body = "<!doctype html>"
    body += "<form action=/ method=post>"
    body += "<p>Username: <input name=username></p>"
    body += "<p>Password: <input name=password type=password></p>"
    body += "<p><button>Log In</button></p>"
    body += "</form>"
    return body

def do_login(session, params):
    username = params.get("username")
    password = params.get("password")
    if username in LOGINS and LOGINS[username] == password:
        session["user"] = username
        return "200 OK", show_comments(session)
    else:
        out = "<!doctype html>"
        out += "<h1>Invalid password for {}</h1>".format(username)
        return "401 Unauthorized", out

def not_found(url, method):
    out = "<!doctype html>"
    out += "<h1>{} {} not found!</h1>".format(method, url)
    return out

def show_count():
    out = "<!doctype html>"
    out += "<div>Let's count up to 99</div>"
    out += "<div>Output</div>"
    for i in range(100): 
        out += "<p>Extra line {}</p>".format(i)
    out += "<script src='/eventloop.js'></script>"
    return out

def do_request(session, method, url, headers, body):
    if method == "GET" and url == "/":
        return "200 OK", show_comments(session)
    elif method == "GET" and url =="/comment.js":
        with open("comment.js") as f:
            return "200 OK", f.read()
    elif method == "GET" and url =="/comment.css":
        with open("comment.css") as f:
            return "200 OK", f.read()
    elif method == "GET" and url == "/eventloop.js":
        with open("eventloop.js") as f:
            return "200 OK", f.read()
    elif method == "GET" and url == "/login":
        return "200 OK", login_form(session)
    elif method == "GET" and url == "/count":
        return "200 OK", show_count()
    elif method == "POST" and url == "/add":
        params = form_decode(body)
        add_entry(session, params)
        return "200 OK", show_comments(session)
    elif method == "POST" and url == "/":
        params = form_decode(body)
        return do_login(session, params)
    else:
        return "404 Not Found", not_found(url, method)

if __name__ == "__main__":
    # Create a socket and listen for connections.
    s = socket.socket(
        family=socket.AF_INET,
        type=socket.SOCK_STREAM,
        proto=socket.IPPROTO_TCP)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(('', 8000))
    s.listen()

    # Enter a loop that runs once per connection.
    # conx is the socket corresponding to the accepted
    # connection.
    while True:
        conx, addr = s.accept()
        handle_connection(conx)
