// // Example of how to use the CGI class in your webserver

// #include "inc/webserv.hpp"

// bool handle_cgi_request(ParsingRequest& request, std::string& response)
// {
//     // Check if the requested file is a CGI script (e.g., ends with .cgi, .py, .php)
//     std::string uri = request.getStartLine().at("uri");
//     if (uri.find(".cgi") == std::string::npos && 
//         uri.find(".py") == std::string::npos && 
//         uri.find(".php") == std::string::npos)
//     {
//         return false; // Not a CGI request
//     }
    
//     // Create CGI instance
//     CGI cgi;
//     std::map<std::string, std::string> env_vars;
    
//     // Set up environment variables
//     if (!cgi.set_env_var(env_vars, request))
//     {
//         response = "HTTP/1.1 500 Internal Server Error\r\n\r\nFailed to set CGI environment variables";
//         return false;
//     }
    
//     // Execute the CGI script
//     if (!cgi.execute(env_vars))
//     {
//         response = "HTTP/1.1 500 Internal Server Error\r\n\r\nFailed to execute CGI script";
//         return false;
//     }
    
//     // Read the output from the CGI script
//     if (!cgi.read_output())
//     {
//         response = "HTTP/1.1 500 Internal Server Error\r\n\r\nFailed to read CGI output";
//         return false;
//     }
    
//     // Get the CGI output
//     std::string cgi_output = cgi.get_output();
    
//     // Parse CGI output to separate headers and body
//     size_t header_end = cgi_output.find("\r\n\r\n");
//     if (header_end == std::string::npos)
//         header_end = cgi_output.find("\n\n");
    
//     if (header_end != std::string::npos)
//     {
//         std::string cgi_headers = cgi_output.substr(0, header_end);
//         std::string cgi_body = cgi_output.substr(header_end + 4);
        
//         // Build HTTP response
//         response = "HTTP/1.1 200 OK\r\n";
//         response += cgi_headers;
//         if (cgi_headers.find("Content-Length:") == std::string::npos)
//         {
//             response += "\r\nContent-Length: " + std::to_string(cgi_body.length());
//         }
//         response += "\r\n\r\n";
//         response += cgi_body;
//     }
//     else
//     {
//         // No proper headers from CGI, treat entire output as body
//         response = "HTTP/1.1 200 OK\r\n";
//         response += "Content-Type: text/html\r\n";
//         response += "Content-Length: " + std::to_string(cgi_output.length()) + "\r\n";
//         response += "\r\n";
//         response += cgi_output;
//     }
    
//     // Clean up
//     cgi.close_cgi();
    
//     return true;
// }

// // Example CGI script (save as test.cgi and make it executable)
// /*
// #!/bin/bash
// echo "Content-Type: text/html"
// echo ""
// echo "<html><body>"
// echo "<h1>Hello from CGI!</h1>"
// echo "<p>Request Method: $REQUEST_METHOD</p>"
// echo "<p>Query String: $QUERY_STRING</p>"
// echo "<p>Server Name: $SERVER_NAME</p>"
// echo "</body></html>"
// */
