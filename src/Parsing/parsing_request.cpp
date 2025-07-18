#include "../../inc/webserv.hpp"
#include <sstream>

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
	if (method == "GET" || method == "POST" ||
		method == "DELETE" || method == "HEAD")
	{
		return true;
	}
	return false;
}

bool ParsingRequest::checkURI(const std::map<std::string, std::string> &start_line)
{
	const std::string &uri = start_line.at("uri");
	if (uri.empty() || uri[0] != '/')
	{
		std::cerr << "Invalid URI: " << uri << std::endl;
		return false;
	}
	// need more cases to handle im sure
	return true;
}

bool ParsingRequest::checkVersion(const std::map<std::string, std::string> &start_line)
{
	const std::string &version = start_line.at("version");
	if (version != "HTTP/1.0" && version != "HTTP/1.1")
	{
		std::cerr << "Invalid HTTP version: " << version << std::endl;
		return false;
	}
	//i guess this too 
	return true;
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
// but how
// hmmmmmmmm
