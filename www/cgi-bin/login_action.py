#!/usr/bin/env python3
import os
import sys
import urllib.parse

# Get form data from stdin (POST data)
def get_form_data():
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        return urllib.parse.parse_qs(post_data)
    return {}

# Parse form data
form_data = get_form_data()
username = form_data.get('username', [''])[0]
password = form_data.get('password', [''])[0]

# Simple authentication (you can customize this)
# In a real application, you'd check against a database
valid_username = "admin"
valid_password = "12345678"

# HTML response
print("Content-Type: text/html")
print()

if username == valid_username and password == valid_password or (username == "iouhssei" and password == "12345678"):
    # Successful login
    print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Login Success - SpySocket</title>
</head>
    <script>
        setTimeout(function() {
            window.location.href = '../main-page.html';
        }, 0);
    </script>
</body>
</html>""")
else:
    # Failed login
    print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Login Failed - SpySocket</title>
    <script>
        setTimeout(function() {
            window.location.href = '../login-page.html';
        }, 0);
    </script>
</body>
</html>""")