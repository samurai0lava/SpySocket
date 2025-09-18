#include "../../inc/POST.hpp"

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
    }
    return response;
}
