#include "../../inc/webserv.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

// parsing request messages

// Lets break this down to parts (start line)
// 1. Check if the request method is valid
// 2. Check if the request URI is valid
// 3. Check if the request version is valid

// get the start line of the request
// split the start line by spaces
// check if the first part is a valid method
// check if the second part is a valid URI
// check if the third part is a valid version
// if all checks are valid, return true

std::string ParsingRequest::get_start_line(const std::string &request)
{
	// traverse the request string until we find the first \r\n
	size_t pos = request.find("\r\n");
	if (pos == std::string::npos)
	{
		std::cerr << "Invalid request format: No start line found." << std::endl;
		return "";
	}
	std::string start_line = request.substr(0, pos);
	return start_line;
}

std::map<std::string, std::string> ParsingRequest::split_start_line(const std::string &start_line)
{
	std::map<std::string, std::string> parsed_start_line;
	std::istringstream ss(start_line);
	std::string method, uri, version;

	ss >> method >> uri >> version;

	parsed_start_line["method"] = method;
	parsed_start_line["uri"] = uri;
	parsed_start_line["version"] = version;

	return parsed_start_line;
}

bool ParsingRequest::checkMethod(const std::map<std::string, std::string> &start_line)
{
	const std::string &method = start_line.at("method");
	if (method.empty())
	{
		throw BadRequestException("400 Bad Request: HTTP method cannot be empty");
	}
	
	if (method == "GET" || method == "POST" ||
		method == "DELETE" || method == "HEAD")
	{
		return true;
	}
	else if (method == "PUT" || method == "PATCH" ||
			 method == "OPTIONS" || method == "TRACE" || method == "CONNECT")
	{
		throw NotImplementedException("501 Not Implemented: HTTP method '" + method + "' is not implemented by our webserver :(");
	}
	else
	{
		connection_status = 0;
		throw BadRequestException("400 Bad Request: Invalid HTTP method: '" + method + "'");
	}
}

bool ParsingRequest::checkURI(const std::map<std::string, std::string> &start_line)
{
	const std::string &uri = start_line.at("uri");
	
	if (uri.empty())
	{
		connection_status = 0;
		throw BadRequestException("400 Bad Request: URI cannot be empty");
	}
	if (uri[0] != '/')
	{
		connection_status = 0;
		throw BadRequestException("400 Bad Request: URI must start with '/' - got: '" + uri + "'");
	}
	
	// RFC 7230 recommends 8000 characters limit
	if (uri.length() > 8000)
	{
		connection_status = 0;
		throw BadRequestException("400 Bad Request: URI too long (exceeds 8000 characters)");
	}
	for (size_t i = 0; i < uri.length(); ++i)
	{
		char c = uri[i];
		// spaces and control characters
		if (c < 32 || c == 127)
		{
			connection_status = 0;
			throw BadRequestException("400 Bad Request: URI contains invalid control characters");
		}
		if (c == ' ')
		{
			connection_status = 0;
			throw BadRequestException("400 Bad Request: URI contains unencoded spaces");
		}
	}
	return true;
}

bool ParsingRequest::checkVersion(const std::map<std::string, std::string> &start_line)
{
	const std::string &version = start_line.at("version");

	if (version.empty())
	{
		connection_status = 0;
		throw BadRequestException("400 Bad Request: HTTP version cannot be empty");
	}
	if (version == "HTTP/1.0" || version == "HTTP/1.1")
	{
		return true;
	}
	else if (version == "HTTP/2" || version == "HTTP/2.0" || 
			 version == "HTTP/3" || version == "HTTP/3.0")
	{
		throw NotImplementedException("HTTP version '" + version + "' is not supported by this server");
	}
	else
	{
		throw BadRequestException("400 Bad Request: Invalid HTTP version format: '" + version + "'");
	}
	return false;

}

// Now lets check for the header fields
// Read the request fiels into hash table till the new line

// Header fields to handle :
//-------------------------------------
// Host : localhost::8080
// --Content-Length-- used in POST methode to tell how much u have to read from the body
// (Must used if the tramsfer encoding its not chunked)
// --Transfer-Encoding : chunked-- ovverides the content lenght if used together
// --Connection : keep alive or close-- in HTTP/1.1 is by default is keep alive
//-------------------------------------

// first need to get the header field until "\r\n\r\n"
// split the header fields by \r\n
// hash table split the key by the value (:)
// then we parse the info is it valid or not
// whitespace problem
// but how ?
// hmmmmmmmm

std::string ParsingRequest::get_header_fields(const std::string &request)
{
	size_t start_line_end = request.find("\r\n");
	if (start_line_end == std::string::npos)
	{
		std::cerr << "Invalid request format: No start line found." << std::endl;
		return "";
	}
	size_t headers_end = request.find("\r\n\r\n");
	if (headers_end == std::string::npos)
	{
		std::cerr << "Invalid request format: No end of headers found." << std::endl;
		return "";
	}
	size_t headers_start = start_line_end + 2;
	if (headers_start >= headers_end)
	{
		return "";
	}

	std::string headers = request.substr(headers_start, headers_end - headers_start);
	return headers;
}

std::map<std::string, std::string> ParsingRequest::split_header(const std::string &headers)
{
	std::map<std::string, std::string> header_map;
	
	std::istringstream stream(headers);
	std::string line;
	
	while (std::getline(stream, line))
	{
		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.length() - 1);
			
		if (line.empty())
			continue;
			
		size_t colon_pos = line.find(':');
		if (colon_pos == std::string::npos)
		{
			throw BadRequestException("400 Bad Request: Invalid header format - missing colon in line: '" + line + "'");
		}
		std::string key = line.substr(0, colon_pos);
		std::string value = line.substr(colon_pos + 1);
		
		key.erase(0, key.find_first_not_of(" \t"));
		key.erase(key.find_last_not_of(" \t") + 1);
		value.erase(0, value.find_first_not_of(" \t"));
		value.erase(value.find_last_not_of(" \t") + 1);
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);	
		header_map[key] = value;
	}
	return header_map;
}

//check the trannfer encoding

//check the content length
//check the connection status
//check the content type
//check the host















std::map<std::string, std::string> ParsingRequest::handle_request(const std::string &request)
{
	std::map<std::string, std::string> parsed_request;
	
	std::string start_line_str = get_start_line(request);
	if (start_line_str.empty())
	{
		throw BadRequestException("400 Bad Request: No start line found in request");
	}
	
	parsed_request = split_start_line(start_line_str);
	checkMethod(parsed_request);
	checkURI(parsed_request);
	checkVersion(parsed_request);
	
	std::string headers_str = get_header_fields(request);
	if (headers_str.empty())
	{
		throw BadRequestException("400 Bad Request: No headers found in request");
	}
	headers = split_header(headers_str);
	return parsed_request;
}





