#!/usr/bin/env python3

import time
import sys

print("Content-Type: text/html\r\n\r\n")
sys.stdout.flush()
time.sleep(60)
print("<html><body>")
print("<h1>CGI Timeout Test</h1>")
print("<p>If you see this, the server did NOT timeout!</p>")
print("</body></html>")
