#!/bin/bash

# Proper CGI script with correct headers
echo "Content-Type: text/plain"
echo "Content-Length: $(printenv | wc -c)"
echo ""  # Blank line required to separate headers from body

# Body starts here
echo "Environment Variables:"
printenv