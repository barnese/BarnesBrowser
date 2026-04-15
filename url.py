import socket
import ssl

# URL handler for a simple browser. See:
# https://browser.engineering/http.html

class URL:
    # Constructor - parses the URL
    def __init__(self, url):
        self.scheme, url = url.split("://", 1)

        # We only support two schemes for now.
        assert self.scheme in ["http", "https"]

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
        request = "GET {} HTTP/1.0\r\n".format(self.path)
        request += "Host: {}\r\n".format(self.host)
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
# END of URL

# Displays an HTML page, very simply.
# def show(body):
#     in_tag = False
#     for c in body:
#         if c == "<":
#             in_tag = True
#         elif c == ">":
#             in_tag = False
#         elif not in_tag:
#             print(c, end="")

# def load(url):
#     body = url.request()
#     show(body)

# Main
# Run examples: 
# python3 browser.py http://example.org/
# python3 browser.py https://example.org/
# python3 browser.py http://localhost:8000
#if __name__ == "__main__":
#    import sys
#    load(URL(sys.argv[1]))
