#!bin/bash

echo "Content-Type: text/html"
echo ""

echo "This is a simple BASH script running from CGI in our Webserv-42"
echo "" 
echo "Current time is $(data)"
echo ""

cat << EOF
<html>

    <head>
        <title> BASH cgi Script </title>
    </head>
    <body>
        <h1>Hello World this is a website running in CGI</h1>
    </body>

</html>

EOF
