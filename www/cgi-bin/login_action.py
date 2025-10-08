#!/usr/bin/env python3
import cgi
import os
import sys

# Get form data
form = cgi.FieldStorage()
username = form.getvalue('username', '')
password = form.getvalue('password', '')
remember = form.getvalue('remember', '')

# Simple authentication (you can customize this)
# In a real application, you'd check against a database
valid_username = "iouhssei"
valid_password = "12345678"

# HTML response
print("Content-Type: text/html")
print()

if username == valid_username and password == valid_password:
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
