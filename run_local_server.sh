#! /bin/sh

# Basic HTTP/1.0 server
#python3 -m http.server 8000 -d assets/test_pages

# HTTP/1.1 with keep-alive support
python3 -m http.server 8000 --bind 127.0.0.1 --directory assets/test_pages --protocol HTTP/1.1

