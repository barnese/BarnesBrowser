import socket
import ssl

# URL handler for a simple browser. See:
# https://browser.engineering/http.html

class URL:
    # Constructor - parses the URL, with optional headers for the request.
    def __init__(self, url, headers=None):
        self.scheme, url = url.split("://", 1)
        self.headers = headers or {}

        # Supported schemes.
        assert self.scheme in ["http", "https", "file"]

        if self.scheme == "http":
            self.port = 80
        elif self.scheme == "https":
            self.port = 443

        if "/" not in url:
            url = url + "/"
        self.host, url = url.split("/", 1)
        self.path = "/" + url

        if ":" in self.host:
            self.host, port = self.host.split(":", 1)
            self.port = int(port)
    
    def request(self):
        if self.scheme == "file":
            return self.read_file()
        else:
            return self.tcp_request()

    def read_file(self):
        with open(self.path, "r") as f:
            data = f.read()
        return data

    def tcp_request(self):
        # Connect to the server via a socket.
        s = socket.socket(
            family=socket.AF_INET, # address family
            type=socket.SOCK_STREAM, # used to send arbitrary amounts of data
            proto=socket.IPPROTO_TCP, # protocol used to establish a connection
        )

        s.connect((self.host, self.port))

        # Wrap the socket with the SSL library for https requests.
        if self.scheme == "https":
            ctx = ssl.create_default_context()
            s = ctx.wrap_socket(s, server_hostname=self.host)

        # Send a get request to the server.
        request = "GET {} HTTP/1.1\r\n".format(self.path)
        request += "Host: {}\r\n".format(self.host)
        request += "Connection: close\r\n"
        request += "User-Agent: BarnesBrowser\r\n"

        for key, value in self.headers.items():
            request += "{}: {}\r\n".format(key, value)

        request += "\r\n"
        s.send(request.encode("utf8"))

        # Wait and receive a response.
        # makefile returns a file-like object containing the server's response.
        response = s.makefile("r", encoding="utf8", newline="\r\n")

        # Parse the response's status line.
        statusline = response.readline()
        version, status, explanation = statusline.split(" ", 2)

        # Parse the response headers.
        response_headers = {}
        while True:
            line = response.readline()
            if line == "\r\n": break
            header, value = line.split(":", 1)
            # casefold is like to-lower case, but works for more languages.
            response_headers[header.casefold()] = value.strip() 

        assert "transfer-encoding" not in response_headers
        assert "content-encoding" not in response_headers

        # The sent data is everything after the headers.
        content = response.read()

        s.close()

        return content
