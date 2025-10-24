#!/usr/bin/env python3

# This script has a syntax error on purpose to test 502 Bad Gateway

print("Content-Type: text/html")
print()
print("<html><body>")

# Syntax error: missing closing parenthesis
print("This will fail"

print("</body></html>")
