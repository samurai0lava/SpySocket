#include "../../inc/POST.hpp"
#include "../../inc/webserv.hpp"

std::string generate_filename(std::string type, std::string termination)
{
    std::string result;
    static int counter = 0;
    result = type;
    result += ft_random_time();
    if(termination.empty())
        result += ".txt";
    else
        result += termination;
    counter++;
    return result;
}


std::string handle_upload(LocationStruct& location, ParsingRequest& parser, ConfigStruct& config)
{
    if (location.upload_enabled == false)
    {
        // return forbidden_403();
		return getErrorPageFromConfig(403, config);
    }
    if (location.upload_path.empty())
        location.upload_path = location.root;
    std::string line;

    std::string boundary = parser.getHeaders().at("boundary");
    std::string request = parser.getBody();

    size_t body_start = request.find("--" + boundary);
    if (body_start == std::string::npos)
    {
        // return bad_request();
		return getErrorPageFromConfig(400, config);
    }
    size_t body_end = request.find("--" + boundary + "--", body_start);
    if (body_end == std::string::npos)
    {
        // return bad_request();
		return getErrorPageFromConfig(400, config);
    }
    std::string body = request.substr(body_start + boundary.length() + 4, body_end - body_start - boundary.length() - 4);
    size_t fn_pos = body.find("filename=");
    int quoted = 0;
	std::string filename;
    //check if no filename provided something sus will happen here
    if (fn_pos == std::string::npos)
    {
        //no error maybe it's just a text not a file (username e.g)
        filename = generate_filename("upload_", "");
    }
	else
	{
		if(body[fn_pos + 9] == '"')
    	{
        	quoted = 1;
    	}

    	fn_pos += 9 + quoted;
		//just added check ittttt
		body = body.substr(fn_pos);
    	size_t fn_end;
    	if(quoted)
    	{
        	fn_end = body.find('"', fn_pos);
        	if (fn_end == std::string::npos)
        	{
            	// return bad_request();
				return getErrorPageFromConfig(400, config);
        	}
    	}
    	else
		{
		//body.find() ??? what if there was already a space before the filename??
			fn_end = body.find(' ', fn_pos);
		}
		filename = body.substr(fn_pos, fn_end - fn_pos);
    	if (filename.empty())
        	filename = generate_filename("upload_", "");
	}
    size_t content_start = body.find("\r\n\r\n");
    if (content_start == std::string::npos)
    {
        // return bad_request();
		return getErrorPageFromConfig(400, config);
	}
    content_start += 4;

    struct stat st;
    if (stat(location.upload_path.c_str(), &st) == 0)
    {
        if (S_ISDIR(st.st_mode))
        {
            filename = location.upload_path + "/" + filename;
        }
        else
        {
            filename = location.upload_path;
        }
    }
    else
    {
        std::cout << location.upload_path << std::endl;
        // return internal_error();
		return getErrorPageFromConfig(500, config);
    }
    std::fstream file(filename.c_str(), std::ios::out);
    if (!file)
    {
        // return internal_error();
		return getErrorPageFromConfig(500, config);
    }

    file.write(body.c_str() + content_start, body.size() - content_start);
    if (!file)
    {
        // return internal_error();
		return getErrorPageFromConfig(500, config);
    }
    file.close();
    // return created_success();
	return getErrorPageFromConfig(201, config);
}

std::vector<std::string> split(std::string s, std::string delimiters)
{
	std::vector<std::string> tokens;
	std::string token;
	//"this is a much longer body than expected"
	for (size_t i = 0; i < s.length(); i++)
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

std::string handle_url_encoded(LocationStruct &location, ParsingRequest &parser, ConfigStruct& config)
{
    std::string body = parser.getBody();

    std::vector<std::string> tokens = split(body, "&");

	std::vector<std::string> pairs;
    for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); it++)
    {
		pairs = split((*it), "=");
        location.url_encoded.insert(std::make_pair(pairs.size() > 0 ? pairs[0] : "No key",  pairs.size() > 1 ? pairs[1] : "No val"));
    }
    std::string res_body = "";
    for (std::map<std::string, std::string>::iterator it = location.url_encoded.begin(); it != location.url_encoded.end(); it++)
        res_body += (*it).first + " : " + (*it).second + "\r\n";

    // return OK_200(res_body);
	return getErrorPageFromConfig(200, config);
}

std::string main_response(LocationStruct &location, ParsingRequest &parser, ConfigStruct& config)
{
    std::string body = parser.getBody();
    std::string filename = "";
    if (parser.getHeaders()["content-type-value"].find("image") != std::string::npos)
    {
        filename = generate_filename("image_", ".png");
    }
    else if (parser.getHeaders()["content-type-value"].find("video") != std::string::npos)
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
        if (S_ISDIR(st.st_mode))
        {
            filename = location.upload_path + "/" + filename;
        }
        else
        {
            filename = location.upload_path;
        }
    }
    else
    {
        // return internal_error();
		return getErrorPageFromConfig(500, config);
    }

    std::fstream file(filename.c_str(), std::ios::binary | std::ios::out);
    if (!file)
    {
        // return internal_error();
		return getErrorPageFromConfig(500, config);
    }

    file.write(body.c_str(), body.length());
    if (!file)
    {
        // return internal_error();
		return getErrorPageFromConfig(500, config);
    }
    file.close();
    // return created_success();
	return getErrorPageFromConfig(201, config);
}

std::string postMethod(std::string uri, ConfigStruct config,
    ParsingRequest& parser)
{
    std::string response = "";

	std::pair<std::string, LocationStruct> location = get_location(uri,
		config);
	if(location.first.empty())
		return getErrorPageFromConfig(404, config);
		// return notFound();
	if(!location.second._return.empty())
	{
		return handle_redirect(location);
		// return getErrorPageFromConfig(301, config); //But there is no redirection message in getErrorPageFromConfig
	}

	if(location.second.allowedMethods.find("POST") == location.second.allowedMethods.end())
	{
		// return handle_notAllowed(location);
		return getErrorPageFromConfig(405, config);
	}

	if (parser.getHeaders()["content-type-value"] == "multipart/form-data")
	{
		response = handle_upload(location.second, parser, config);
	}
	else if(parser.getHeaders()["content-type-value"] == "application/x-www-form-urlencoded")
	{
		response = handle_url_encoded(location.second, parser, config);
	}
	else
		response = main_response(location.second, parser, config);
    return response;
}
