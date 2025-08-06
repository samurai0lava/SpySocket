CGI stand for Common gateway interface, its solve a problem, back in the days servers can only serve static website (HTML , IMAGES and fixed content)
but what i f i want to run something in my website like changing news or a weather stats...
CGI run that program and return that program output as a web page.

**How CGI works ?**

The server get a request and recognizes its for a CGI , setups the env variables containing all the request information, then the server launches the CGI program as a separate Process , and finally takes this output and send it back to the users browser.

**Environment Variables**:
CGI communicates through env variables , includes *REQUEST_METHOD* , *QUERY_STRING*, ...

CGI has its limitations since creates a new process for each request , today servers use more efficient alternatives like *FastCGI*, *WSGI* (for python).

**FastCGI, How its works ?**

For traditional CGI its create every time a new process , FastCGI solves this problem by keeping processes alive and reusing them, when it receive a request it looks at its pool of existing FastCGI processes.
The process manager it can dynamically adjust the number of processes based on the load 
FastCGI dont have to run at the same machine as the webserver , because FastCGI communicate through its network protocol.


**--How Server knows its a CGI script and it needs to execute it ?**

in two ways , first treat the file like any other file checking , with its extension (``.cgi`` or ``.exe``), if the file end with those extension , than it will pass to the CGI to executed.
the second method is treating everything in that ``cgi-bin`` directory an executable.

**-- URL encoding**

```
http://example.com/cgi-bin/search.cgi/category/books?query=python&author=smith
```

``/cgi-bin/search.cgi``  ---> tells the server what program to execute.
``/category/books``      ---> provides additional context that the program can use.
``?query=python&author=smith`` ---> carries variable data.
    


**How i can implement it in my WEBSERV ?**

The CGI implementation follows a standard Unix process model with pipe-based communication.
1. Setting up CGI environment variables from the HTTP request (method, headers, paths, etc.).
	-- By encoding the URI , because we need the script path ,the query value and 
	the data need for the script to run.
	--But we have a small issue , when we know its a CGI script, well there is a solution treats any file that inside the  ```cgi-bin``` folder a script.
	--we check for the interpreter for that script extension. or if has no extension or ```.cgi``` , we get it from the shebang in the script.
2. Creates two pipes - one for input to the CGI script and one for output from it. It forks a child process where the child redirects its STDIN/STDOUT to the pipes,
3. Executes the CGI script using `execve()` with the appropriate interpreter 
4. The parent process manages the pipes, handles timeout scenarios using `select()` with interval-based counting.
5. Reads the CGI output in a non-blocking manner. 