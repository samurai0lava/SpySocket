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
		logError(400, "Bad Request: HTTP method cannot be empty");
		return false;
	}
	
	if (method == "GET" || method == "POST" ||
		method == "DELETE" || method == "HEAD")
	{
		return true;
	}
	else if (method == "PUT" || method == "PATCH" ||
			 method == "OPTIONS" || method == "TRACE" || method == "CONNECT")
	{
		connection_status = 1;
		logError(501, "Not Implemented: HTTP method '" + method + "' is not implemented by our webserver :(");
		return false;
	}
	else
	{
		connection_status = 0;
		logError(400, "Bad Request: Invalid HTTP method: '" + method + "'");
		return false;
	}
	return false;
}

bool ParsingRequest::checkURI(const std::map<std::string, std::string> &start_line)
{
	const std::string &uri = start_line.at("uri");
	
	if (uri.empty())
	{
		connection_status = 0;
		logError(400, "Bad Request: URI cannot be empty");
		return false;
	}
	if (uri[0] != '/')
	{
		connection_status = 0;
		logError(400, "Bad Request: URI must start with '/' - got: '" + uri + "'");
		return false;
	}
	
	// RFC 7230 recommends 8000 characters limit
	if (uri.length() > 8000)
	{
		connection_status = 0;
		logError(400, "Bad Request: URI too long (exceeds 8000 characters)");
		return false;
	}
	for (size_t i = 0; i < uri.length(); ++i)
	{
		char c = uri[i];
		// spaces and control characters
		if (c < 32 || c == 127)
		{
			connection_status = 0;
			logError(400, "Bad Request: URI contains invalid control characters");
			return false;
		}
		if (c == ' ')
		{
			connection_status = 0;
			logError(400, "Bad Request: URI contains unencoded spaces");
			return false;
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
		logError(400, "Bad Request: HTTP version cannot be empty");
		return false;
	}
	if (version == "HTTP/1.0" || version == "HTTP/1.1")
	{
		return true;
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
			connection_status = 0;
			logError(400, "Bad Request: Invalid header format - no colon found in line: '" + line + "'");
		}
		
		std::string key = line.substr(0, colon_pos);
		std::string value = line.substr(colon_pos + 1);
		
		if (!key.empty() && (key[key.length() - 1] == ' ' || key[key.length() - 1] == '\t'))
		{
			connection_status = 0;
			logError(400, "Bad Request: Invalid header name - trailing whitespace not allowed: '" + key + "'");
		}
		if (!key.empty() && (key[0] == ' ' || key[0] == '\t'))
		{
			connection_status = 0;
			logError(400, "Bad Request: Invalid header name - leading whitespace not allowed: '" + key + "'");
		}
		
		// (must be tokens per RFC 7230)
		for (size_t i = 0; i < key.length(); ++i)
		{
			char c = key[i];
			if (c < 33 || c > 126 || c == '(' || c == ')' || c == '<' || c == '>' || 
				c == '@' || c == ',' || c == ';' || c == ':' || c == '\\' || 
				c == '"' || c == '/' || c == '[' || c == ']' || c == '?' || 
				c == '=' || c == '{' || c == '}')
			{
				connection_status = 0;
				logError(400, "Bad Request: Invalid character in header name: '" + std::string(1, c) + "'");
			}
		}
		if (key.empty())
		{
			connection_status = 0;
			logError(400, "Bad Request: Empty header name not allowed");
		}
		
		value.erase(0, value.find_first_not_of(" \t"));
		value.erase(value.find_last_not_of(" \t") + 1);
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);	
		std::transform(value.begin(), value.end(), value.begin(), ::tolower);
		header_map[key] = value;
	}
	return header_map;
}


//check the content length (DOONE)
//check the connection status (DOONE)
//check the host (DOONE)

//check the content type
//check the transfer encoding


bool ParsingRequest::checkTransferEncoding(const std::map<std::string, std::string> &headers)
{
	//check the Transfer Encodue value
	//if is other than chunked return a NotImplementedException
	//and if its chunked set the transfer encoding exists and its chunked
	if (headers.find("transfer-encoding") != headers.end())
	{
		transfer_encoding_exists = 1;
		std::string transfer_value;
		if (transfer_value == "gzip" || transfer_value == "deflate" || transfer_value == "compress")
		{
			logError(501, "Not Implemented: HTTP Tranfer-Encoding value not supported");
			return false;
		}
		else
		{
			logError(400, "Bad Request: HTTP Tranfer-Encoding value does not exist");
			return false;
		}
	}
	transfer_encoding_exists = 0;
	return (true);
}

bool ParsingRequest::checkConnection(const std::map<std::string, std::string> &headers)
{
	//check the Connection value
	//if exists set the connection status to 1
	//if not exists set the connection status to 0
	// default is keep-alive for HTTP/1.1
	if (headers.find("connection") != headers.end())
	{
		std::string connection_value = headers.at("connection");
		if (connection_value == "keep-alive")
		{
			connection_status = 1;
			return true;
		}
		else if (connection_value == "close")
		{
			connection_status = 0;
			return true;
		}
		else
		{
			connection_status = 0;
			logError(400, "Bad Request: Invalid Connection header value - must be 'keep-alive' or 'close'");
			return false;
		}
	}
	connection_status = 1;
	return true;
}

bool ParsingRequest::checkContentLength(const std::map<std::string, std::string> &headers)
{
	//check the Content Length value
	//if exists set the content length exists to 1
	//if not exists set the content length exists to 0
	if (headers.find("content-length") != headers.end())
	{
		content_lenght_exists = 1;
		std::string content_length_str = headers.at("content-length");
		if (content_length_str.empty())
		{
			connection_status = 0;
			logError(400, "Bad Request: Content-Length header cannot be empty");
			return false;
		}
		for (size_t i = 0; i < content_length_str.length(); ++i)
		{
			if (!isdigit(content_length_str[i]))
			{
				connection_status = 0;
				logError(400, "Bad Request: Content-Length header must be a valid integer - got: '" + content_length_str + "'");
				return false;
			}
		}
		int content_length;
		std::istringstream iss(content_length_str);
		iss >> content_length;
		if (iss.fail() || !iss.eof())
		{  
			connection_status = 0;
			logError(400, "Bad Request: Content-Length header must be a valid integer - got: '" + content_length_str + "'");
			return false;
		}
		if (content_length < 0)
		{
			connection_status = 0;
			logError(400, "Bad Request: Content-Length header cannot be negative - got: '" + content_length_str + "'");
			return false;
		}
		if (content_length > 8000)
		{
			connection_status = 0;
			logError(400, "Bad Request: Content-Length header exceeds maximum allowed size (8000 bytes) - got: '" + content_length_str + "'");
			return false;
		}
		return true;
	}
	content_lenght_exists = 0;
	return true;
}

bool ParsingRequest::checkHost(const std::map<std::string, std::string>& headers)
{
	if (headers.find("host") != headers.end())
	{
		host_exists = 1;
		return true;
	}
	host_exists = 0;
	connection_status = 0;
	logError(400, "Bad Request: Host header is missing");
	return false;
}

bool ParsingRequest::handle_request(const std::string &request)
{
	std::string start_line_str = get_start_line(request);
	if (start_line_str.empty())
	{
		logError(400, "Bad Request: No start line found in request");
		return false; 
	}
	
	start_line = split_start_line(start_line_str);
	if(checkURI(start_line) == false || checkMethod(start_line) == false || checkVersion(start_line) == false)
	{
		start_line.clear();
		return false; 
	}
	
	std::string headers_str = get_header_fields(request);
	if (headers_str.empty())
	{
		logError(400, "Bad Request: No headers found in request");
		return false; 
	}
	
	headers = split_header(headers_str);
	if(checkConnection(headers) == false || checkHost(headers) == false || checkContentLength(headers) == false)
	{
		headers.clear(); 
		return false; 
	}
	
	return true;
}

