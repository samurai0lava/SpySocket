#!/usr/bin/env python3

import sys
import os

# Print HTTP headers
print("Content-Type: text/html")
print("")

# Get method and content length
method = os.environ.get('REQUEST_METHOD', 'UNKNOWN')
content_length = int(os.environ.get('CONTENT_LENGTH', '0'))

print("<html><body>")
print(f"<h1>CGI Test</h1>")
print(f"<p>Method: {method}</p>")
print(f"<p>Content Length: {content_length}</p>")

if method == 'POST' and content_length > 0:
    # Read POST data
    post_data = sys.stdin.read(content_length)
    print(f"<p>POST Data: {post_data}</p>")

print("</body></html>")
