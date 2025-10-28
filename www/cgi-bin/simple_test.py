#!/usr/bin/env python3

import os
import sys

print("Content-Type: text/html\r")
print("\r")  # Empty line required after headers

print("""
<!DOCTYPE html>
<html>
<head>
    <title>Python CGI Test</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .env-var { margin: 5px 0; padding: 5px; background-color: #f0f0f0; }
    </style>
</head>
<body>
    <h1>Python CGI Test Script</h1>
    <h2>Environment Variables:</h2>
""")

env_vars = ['REQUEST_METHOD', 'SCRIPT_NAME', 'PATH_INFO', 'QUERY_STRING',
           'CONTENT_TYPE', 'CONTENT_LENGTH', 'SERVER_NAME', 'SERVER_PORT',
           'HTTP_USER_AGENT', 'HTTP_HOST', 'GATEWAY_INTERFACE']

for var in env_vars:
    value = os.environ.get(var, '(not set)')
    print(f'    <div class="env-var"><strong>{var}:</strong> {value}</div>')

print("""
    <h2>POST Data (if any):</h2>
""")

if os.environ.get('REQUEST_METHOD') == 'POST':
    content_length = os.environ.get('CONTENT_LENGTH')
    if content_length:
        post_data = sys.stdin.read(int(content_length))
        print(f'    <div class="env-var">POST Data: {post_data}</div>')
    else:
        print('    <div class="env-var">No POST data</div>')
else:
    print('    <div class="env-var">Not a POST request</div>')

print("""
</body>
</html>
""")
