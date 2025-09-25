#!/usr/bin/env python3
import time
import sys

# CGI must output a Content-Type header first
print("Content-Type: text/html\r\n\r\n")

# Simulate a process that takes too long
sys.stdout.flush()  # flush headers immediately
time.sleep(60)      # sleep 60 seconds (adjust to your server timeout)

# This part will never be seen if server kills it before finishing
print("<html><body>")
print("<h1>CGI Timeout Test</h1>")
print("<p>If you see this, the server did NOT timeout!</p>")
print("</body></html>")
