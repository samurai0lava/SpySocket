#!/usr/bin/env python3

import sys
import os

print("Content-Type: text/html")
print("")

print("<html><body>")
print("<h1>CGI POST Debug</h1>")

# Debug environment variables
print(f"<p>REQUEST_METHOD: {os.environ.get('REQUEST_METHOD', 'UNSET')}</p>")
print(f"<p>CONTENT_LENGTH: {os.environ.get('CONTENT_LENGTH', 'UNSET')}</p>")
print(f"<p>CONTENT_TYPE: {os.environ.get('CONTENT_TYPE', 'UNSET')}</p>")

# Try to read POST data
content_length = os.environ.get('CONTENT_LENGTH')
if content_length and content_length.isdigit():
    content_length = int(content_length)
    if content_length > 0:
        try:
            post_data = sys.stdin.read(content_length)
            print(f"<p>POST Data Length: {len(post_data)}</p>")
            print(f"<p>POST Data: <pre>{post_data}</pre></p>")
        except Exception as e:
            print(f"<p>Error reading POST data: {e}</p>")
    else:
        print("<p>Content length is 0</p>")
else:
    print("<p>No content length set</p>")

# Try cgi.FieldStorage()
try:
    import cgi
    form = cgi.FieldStorage()
    print(f"<p>Form keys: {list(form.keys())}</p>")
    for key in form.keys():
        print(f"<p>{key}: {form.getvalue(key)}</p>")
except Exception as e:
    print(f"<p>Error with FieldStorage: {e}</p>")

print("</body></html>")
