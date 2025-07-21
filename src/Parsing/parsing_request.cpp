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


// bool ParsingRequest::checkTransferEncoding(const std::map<std::string, std::string> &headers)
// {
// 	//check the Transfer Encodue value
// 	//if is other than chunked return a NotImplementedException
// 	//and if its chunked set the transfer encoding exists and its chunked
// 	if (headers.find("transfer-encoding") != headers.end())
// 	{
// 		transfer_encoding_exists = 1;
// 		std::string transfer_value;
// 		if (transfer_value == "gzip" || transfer_value == "deflate" || transfer_value == "compress")
// 		{
// 			logError(501, "Not Implemented: HTTP Tranfer-Encoding value not supported");
// 			return false;
// 		}
// 		else
// 		{
// 			logError(400, "Bad Request: HTTP Tranfer-Encoding value does not exist");
// 			return false;
// 		}
// 	}
// 	transfer_encoding_exists = 0;
// 	return (true);
// }

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

// // NGINX-style incremental parsing implementation
// ParsingRequest::ParseResult ParsingRequest::feed_data(const char* data, size_t len)
// {
// 	buffer.append(data, len);
	
// 	// State machine loop
// 	while (current_state != PARSE_COMPLETE && current_state != PARSE_ERROR)
// 	{
// 		switch (current_state)
// 		{
// 			case PARSE_START_LINE:
// 				if (!parse_start_line())
// 					return PARSE_AGAIN; // Need more data
// 				current_state = PARSE_HEADERS;
// 				break;
				
// 			case PARSE_HEADERS:
// 				if (!parse_headers())
// 					return PARSE_AGAIN; // Need more data
// 				// Check if we need to parse body
// 				if (content_lenght_exists && expected_body_length > 0)
// 					current_state = PARSE_BODY;
// 				else
// 					current_state = PARSE_COMPLETE;
// 				break;
				
// 			case PARSE_BODY:
// 				if (!parse_body())
// 					return PARSE_AGAIN; // Need more data
// 				current_state = PARSE_COMPLETE;
// 				break;
				
// 			default:
// 				break;
// 		}
// 	}
	
// 	if (current_state == PARSE_ERROR)
// 		return PARSE_ERROR_400;
	
// 	return PARSE_OK;
// }

// bool ParsingRequest::parse_start_line()
// {
// 	size_t crlf_pos;
// 	if (!find_crlf(crlf_pos))
// 		return false; // Need more data
	
// 	// Extract start line
// 	std::string start_line_str = buffer.substr(buffer_pos, crlf_pos - buffer_pos);
// 	buffer_pos = crlf_pos + 2; // Skip CRLF
	
// 	// Parse start line
// 	start_line = split_start_line(start_line_str);
	
// 	// Validate start line
// 	try {
// 		if (!checkMethod(start_line) || !checkURI(start_line) || !checkVersion(start_line))
// 		{
// 			current_state = PARSE_ERROR;
// 			return false;
// 		}
// 	} catch (...) {
// 		current_state = PARSE_ERROR;
// 		return false;
// 	}
	
// 	return true;
// }

// bool ParsingRequest::parse_headers()
// {
// 	std::string headers_str;
	
// 	// Find end of headers (double CRLF)
// 	size_t double_crlf = buffer.find("\r\n\r\n", buffer_pos);
// 	if (double_crlf == std::string::npos)
// 		return false; // Need more data
	
// 	// Extract headers
// 	headers_str = buffer.substr(buffer_pos, double_crlf - buffer_pos);
// 	buffer_pos = double_crlf + 4; // Skip double CRLF
	
// 	// Parse headers
// 	headers = split_header(headers_str);
	
// 	// Validate headers
// 	try {
// 		if (!checkHost(headers) || !checkConnection(headers))
// 		{
// 			current_state = PARSE_ERROR;
// 			return false;
// 		}
		
// 		// Check content length for body parsing
// 		if (checkContentLength(headers) && content_lenght_exists)
// 		{
// 			std::string content_length_str = headers.at("content-length");
// 			std::istringstream iss(content_length_str);
// 			iss >> expected_body_length;
// 		}
// 	} catch (...) {
// 		current_state = PARSE_ERROR;
// 		return false;
// 	}
	
// 	return true;
// }

// bool ParsingRequest::parse_body()
// {
// 	size_t available = buffer.length() - buffer_pos;
// 	if (available < expected_body_length)
// 		return false;
// 	body_content = buffer.substr(buffer_pos, expected_body_length);
// 	buffer_pos += expected_body_length;
	
// 	return true;
// }

// bool ParsingRequest::find_crlf(size_t& pos)
// {
// 	pos = buffer.find("\r\n", buffer_pos);
// 	return (pos != std::string::npos);
// }

// ParsingRequest::ParseResult ParsingRequest::get_parse_status() const
// {
// 	switch (current_state)
// 	{
// 		case PARSE_COMPLETE:
// 			return PARSE_OK;
// 		case PARSE_ERROR:
// 			return PARSE_ERROR_400;
// 		default:
// 			return PARSE_AGAIN;
// 	}
// }

// void ParsingRequest::reset()
// {
// 	current_state = PARSE_START_LINE;
// 	buffer.clear();
// 	buffer_pos = 0;
// 	expected_body_length = 0;
// 	body_content.clear();
// 	start_line.clear();
// 	headers.clear();
// 	connection_status = 1;
// 	content_lenght_exists = 0;
// 	transfer_encoding_exists = 0;
// 	host_exists = 0;
// }

