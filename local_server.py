from functools import partial
from http.server import HTTPServer, SimpleHTTPRequestHandler

class Handler(SimpleHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def end_headers(self):
        # Add a cache control header to test caching (60 second max age)
        self.send_header("Cache-Control", "max-age=60")
        super().end_headers()

handler = partial(Handler, directory="assets/test_pages")
server = HTTPServer(("127.0.0.1", 8000), handler)
server.serve_forever()
