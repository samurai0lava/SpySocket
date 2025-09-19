#include "../../inc/POST.hpp"
#include "../../inc/webserv.hpp"

std::string generate_filename(string type, string termination)
{
    std::string result;
    static int counter = 0;
    result = type;
    result += (char)(counter + '0');
    if(termination.empty())
        result += ".txt";
    else    
        result += termination;
    counter++;
    return result;
}


string handle_upload(LocationStruct& location, ParsingRequest& parser)
{
    if (location.upload_enabled == false)
    {
        return forbidden_403();
    }
    if (location.upload_path.empty())
        location.upload_path = location.root;
    std::string line;

    std::string boundary = parser.getHeaders().at("boundary");
    std::string request = parser.getBody();

    // cout << "BODY :::::: " << request << "BODY ENDDDDDD\n";
    size_t body_start = request.find("--" + boundary);
    if (body_start == std::string::npos)
    {
        return bad_request();
    }
    size_t body_end = request.find("--" + boundary + "--", body_start);
    if (body_end == std::string::npos)
    {
        return bad_request();
    }
    std::string body = request.substr(body_start + boundary.length() + 4, body_end - body_start - boundary.length() - 4);
    size_t fn_pos = body.find("filename=");
    int quoted = 0;
    if(body[fn_pos + 9] == '"')
    {
        quoted = 1;
    }
    if (fn_pos == std::string::npos)
    {
        //no error maybe it's just a text not a file (username e.g)
        std::cerr << "Error: No filename found.\n";
        // return;
    }
    fn_pos += 9 + quoted;
    size_t fn_end;
    if(quoted)
    {
        fn_end = body.find('"', fn_pos);
        if (fn_end == std::string::npos)
        {
            return bad_request();
        }
    }
    else
        fn_end = body.find(' ', fn_pos);
    std::string filename = body.substr(fn_pos, fn_end - fn_pos);
    if (filename.empty())
        filename = generate_filename("upload_", "");

    size_t content_start = body.find("\r\n\r\n");
    if (content_start == std::string::npos)
    {
        return bad_request();
    }
    content_start += 4; // Skip past the "\r\n\r\n"

    struct stat st;
    if (stat(location.upload_path.c_str(), &st) == 0)
    {
        // it's a directory
        if (S_ISDIR(st.st_mode)) 
        {
            //pay ATTENTION to the slash "/"
            filename = location.upload_path + "/" + filename;
        }
        else 
        {
            filename = location.upload_path;
        }
    }
    // else some error occured with stat
    else
    {
        cout << location.upload_path << endl;
        cout << RED "111111111111111\n" RESET ;
        return internal_error();
    }
    std::fstream file(filename.c_str(), std::ios::out);
    if (!file)
    {
        return internal_error();
    }

    file.write(body.c_str() + content_start, body.size() - content_start);
    if (!file)
    {
        return internal_error();
    }
    file.close();
    return created_success();
}

std::vector<std::string> split(std::string s, std::string delimiters)
{
	std::vector<std::string> tokens;
	std::string token;
	for (std::string::size_type i = 0; i < s.size(); ++i)
	{
		if (delimiters.find(s[i]) != std::string::npos)
		{
			if (!token.empty())
			{
				tokens.push_back(token);
				token.clear();
			}
		}
		else
		{
			token += s[i];
		}
	}
	if (!token.empty())
	{
		tokens.push_back(token);
	}
	return (tokens);
}

string handle_url_encoded(LocationStruct &location, ParsingRequest &parser)
{
    string body = parser.getBody();

    vector<string> tokens = split(body, "&");
    vector<string> pairs;
    for(vector<string>::iterator it = tokens.begin(); it != tokens.end(); it++)
    {
        pairs = split((*it), "=");
        //check if there is a possiblity that we can assign to "" (e.g name="")
        location.url_encoded.insert(std::make_pair(pairs[0], pairs[1]));
    }
    string res_body = "";
    for(map<string, string>::iterator it = location.url_encoded.begin(); it != location.url_encoded.end(); it++)
        res_body += (*it).first + " : " + (*it).second + "\r\n";

    return OK_200(res_body);
}

string main_response(LocationStruct &location, ParsingRequest &parser)
{
    string body = parser.getBody();
    string filename = "";
    if(parser.getHeaders()["content-type-value"].find("image") != string::npos)
    {
        filename = generate_filename("image_", ".png");
    }
    else if(parser.getHeaders()["content-type-value"].find("video") != string::npos)
    {
        filename = generate_filename("video_", ".mp4");
    }
    else
    {
        filename = generate_filename("file_", ".txt");
    }
    struct stat st;
    if(location.upload_enabled == true && location.upload_path.empty())
    {
        location.upload_path = location.root;
    }
    if (stat(location.upload_path.c_str(), &st) == 0)
    {
        // it's a directory
        if (S_ISDIR(st.st_mode)) 
        {
            //pay ATTENTION to the slash "/"
            filename = location.upload_path + "/" + filename;
        }
        else 
        {
            filename = location.upload_path;
        }
    }
    else
    {
        return internal_error();
    }

    std::fstream file(filename.c_str(), std::ios::binary | std::ios::out);
    if (!file)
    {
        return internal_error();
    }

    file.write(body.c_str(), body.length());
    if (!file)
    {
        return internal_error();
    }
    file.close();
    return created_success();
}

string	postMethod(string uri, ConfigStruct config,
    ParsingRequest& parser)
{
    string response = "";
    
    // Check if this is a CGI request first
    CGI cgi;
    if (cgi.check_is_cgi(parser))
    {
        std::map<std::string, std::string> env_vars;
        if (cgi.set_env_var(env_vars, parser))
        {
            if (cgi.execute_with_body(env_vars, parser.getBody()))
            {
                if (cgi.read_output())
                {
                    std::string cgi_output = cgi.get_output();
                    
                    // Parse CGI output to separate headers and body
                    size_t header_end = cgi_output.find("\r\n\r\n");
                    if (header_end == std::string::npos)
                        header_end = cgi_output.find("\n\n");
                    
                    if (header_end != std::string::npos)
                    {
                        std::string cgi_headers = cgi_output.substr(0, header_end);
                        std::string cgi_body = cgi_output.substr(header_end + (cgi_output.find("\r\n\r\n") != std::string::npos ? 4 : 2));
                        
                        std::ostringstream response_stream;
                        response_stream << "HTTP/1.1 200 OK\r\n";
                        
                        // Check if CGI provided Content-Type, if not add default
                        if (cgi_headers.find("Content-Type:") == std::string::npos)
                        {
                            response_stream << "Content-Type: text/html\r\n";
                        }
                        
                        response_stream << cgi_headers << "\r\n";
                        
                        // Add Content-Length if not provided by CGI
                        if (cgi_headers.find("Content-Length:") == std::string::npos)
                        {
                            response_stream << "Content-Length: " << cgi_body.size() << "\r\n";
                        }
                        
                        response_stream << "\r\n" << cgi_body;
                        
                        return response_stream.str();
                    }
                    else
                    {
                        // No proper headers from CGI, treat as plain output
                        std::ostringstream response_stream;
                        response_stream << "HTTP/1.1 200 OK\r\n";
                        response_stream << "Content-Type: text/html\r\n";
                        response_stream << "Content-Length: " << cgi_output.size() << "\r\n\r\n";
                        response_stream << cgi_output;
                        return response_stream.str();
                    }
                }
                else
                {
                    return GenerateResErr(cgi.get_error_code() != 0 ? cgi.get_error_code() : 500);
                }
            }
            else
            {
                return GenerateResErr(cgi.get_error_code() != 0 ? cgi.get_error_code() : 500);
            }
        }
        else
        {
            return GenerateResErr(500);
        }
    }
    
    // cout << "BODY :::::: " << parser.getBody() << "BODY ENDDDDDD\n";
    try
    {
        std::pair<std::string, LocationStruct> location = get_location(uri,
            config);
        if(location.first.empty())
            return notFound();
        //check if method allowed and check for redirection
        if(!location.second._return.empty())
        {
            return handle_redirect(location);
        }

        if(location.second.allowedMethods.find("POST") == location.second.allowedMethods.end())
        {
            return handle_notAllowed(location);
        }
        

        if (parser.getHeaders()["content-type-value"] == "multipart/form-data")
        {
            response = handle_upload(location.second, parser);
        }
        else if(parser.getHeaders()["content-type-value"] == "application/x-www-form-urlencoded")
        {
            response = handle_url_encoded(location.second, parser);
        }
        else
            response = main_response(location.second, parser);

    }
    catch (exception& e)
    {
        std::string error_msg = e.what();
        if (error_msg.find("Method not allowed") != std::string::npos)
            response = GenerateResErr(405);
        else if (error_msg.find("Location not found") != std::string::npos)
            response = GenerateResErr(404);
        else if (error_msg.find("Redirection") != std::string::npos)
            response = GenerateResErr(302);
        else
            response = internal_error(); // 500 Internal Server Error
    }
    return response;
}
