#include "../../inc/webserv.hpp"


// NGINX-style incremental parsing implementation


int parse_hex(const std::string& s)
{
	std::istringstream iss(s);
	int n;
	iss >> std::uppercase >> std::hex >> n;
	return n;
}

std::string url_Decode(const std::string& s)
{
	std::string result;
	result.reserve(s.size());
	for (std::size_t i = 0; i < s.size();) {
		if (s[i] != '%') {
			result.push_back(s[i]);
			++i;
		}
		else {
			result.push_back(parse_hex(s.substr(i + 1, 2)));
			i += 3;
		}
	}
	return result;
}

std::string normalizePath(const std::string& path)
{
	std::vector<std::string> stack;
	std::istringstream iss(path);
	std::string token;

	while (std::getline(iss, token, '/')) {
		if (token.empty() || token == ".") {
			continue;
		}
		if (token == "..") {
			if (!stack.empty()) {
				stack.pop_back();
			}
		}
		else {
			stack.push_back(token);
		}
	}

	std::string normalized = "/";
	for (size_t i = 0; i < stack.size(); ++i) {
		normalized += stack[i];
		if (i + 1 < stack.size()) {
			normalized += "/";
		}
	}
	return normalized;
}



bool ParsingRequest::checkURI(std::string& uri)
{
	if (uri.empty())
	{
		connection_status = 0;
		error_code = 400;
		error_message = "Bad Request: URI cannot be empty";
		access_error(error_code, error_message);
		current_state = PARSE_ERROR;
		return false;
	}
	if (uri[0] != '/')
	{
		connection_status = 0;
		error_code = 400;
		error_message = "Bad Request: URI must start with '/' - got: '" + uri + "'";
		access_error(error_code, error_message);
		current_state = PARSE_ERROR;
		return false;
	}
	if (uri.length() > 8000)
	{
		connection_status = 0;
		error_code = 414;
		error_message = "URI Too Long: URI exceeds 8000 characters";
		access_error(error_code, error_message);
		current_state = PARSE_ERROR;
		return false;
	}
	for (size_t i = 0; i < uri.length(); ++i)
	{
		char c = uri[i];
		if (c < 32 || c == 127)
		{
			connection_status = 0;
			error_code = 400;
			error_message = "Bad Request: URI contains invalid control characters";
			access_error(error_code, error_message);
			current_state = PARSE_ERROR;
			return false;
		}
		if (c == ' ')
		{
			connection_status = 0;
			error_code = 400;
			error_message = "Bad Request: URI contains unencoded spaces";
			current_state = PARSE_ERROR;
			access_error(error_code, error_message);
			return false;
		}
	}
	std::string decoded_uri = url_Decode(uri);
	uri = normalizePath(decoded_uri);
	return true;
}
bool ParsingRequest::checkVersion(const std::string& version)
{
	if (version.empty())
	{
		connection_status = 0;
		error_code = 400;
		error_message = "Bad Request: HTTP version cannot be empty";
		access_error(error_code, error_message);
		current_state = PARSE_ERROR;
		return false;
	}
	if (version != "HTTP/1.0" && version != "HTTP/1.1")
	{
		connection_status = 0;
		error_code = 400;
		error_message = "Bad Request: Invalid HTTP version: '" + version + "'";
		current_state = PARSE_ERROR;
		access_error(error_code, error_message);
		return false;
	}
	return true;
}

bool ParsingRequest::checkMethod(const std::string& method)
{
	if (method.empty())
	{
		error_code = 400;
		error_message = "Bad Request: HTTP method cannot be empty";
		current_state = PARSE_ERROR;
		access_error(error_code, error_message);
		return false;
	}

	if (method == "PUT" || method == "PATCH" || method == "OPTIONS" || method == "TRACE" || method == "CONNECT")
	{
		error_code = 501;
		error_message = "Not Implemented: HTTP method '" + method + "' is not implemented";
		current_state = PARSE_ERROR;
		result_p = PARSE_ERROR_501;

		access_error(error_code, error_message);
		return false;
	}
	else if (method != "GET" && method != "POST" && method != "DELETE" && method != "HEAD")
	{
 		connection_status = 0;
		error_code = 400;
		error_message = "Bad Request: Invalid HTTP method: '" + method + "'";
		current_state = PARSE_ERROR;
		access_error(error_code, error_message);
		return false;
	}
	return true;
}


bool ParsingRequest::parse_start_line()
{
	size_t crlf_pos;
	if (!find_crlf(crlf_pos))
		return false;

	std::string start_line_str = buffer.substr(buffer_pos, crlf_pos - buffer_pos);
	buffer_pos = crlf_pos + 2;

	std::istringstream ss(start_line_str);
	std::string method, uri, version;
	
	std::getline(ss, method, ' ');
	std::getline(ss, uri, ' ');
	std::getline(ss, version, ' ');
	
	if (!checkMethod(method) || !checkURI(uri) || !checkVersion(version))
		return false;

	start_line["method"] = method;
	start_line["uri"] = uri;
	start_line["version"] = version;

	return true;
}


bool ParsingRequest::checkLocation(const std::map<std::string, std::string>& headers)
{
	if (headers.find("location") != headers.end())
	{
		std::string location = headers.at("location");
		if (location.empty())
		{
			connection_status = 0;
			error_code = 400;
			error_message = "Bad Request: Location header cannot be empty";
			current_state = PARSE_ERROR;
			access_error(error_code, error_message);
			return false;
		}
		if (location[0] != '/')
		{
			connection_status = 0;
			error_code = 400;
			error_message = "Bad Request: Location header must start with '/' - got: '" + location + "'";
			current_state = PARSE_ERROR;
			access_error(error_code, error_message);
			return false;
		}
		if (location.length() > 8000)
		{
			connection_status = 0;
			error_code = 400;
			error_message = "Bad Request: Location header too long (exceeds 8000 characters)";
			current_state = PARSE_ERROR;
			access_error(error_code, error_message);
			return false;
		}
		for (size_t i = 0; i < location.length(); ++i)
		{
			char c = location[i];
			if (c < 32 || c == 127)
			{
				connection_status = 0;
				error_code = 400;
				error_message = "Bad Request: Location header contains invalid control characters";
				current_state = PARSE_ERROR;
				access_error(error_code, error_message);
				return false;
			}
			if (c == ' ')
			{
				connection_status = 0;
				error_code = 400;
				error_message = "Bad Request: Location header contains unencoded spaces";
				current_state = PARSE_ERROR;
				access_error(error_code, error_message);
				return false;
			}
		}
	}
	return true;
}

bool ParsingRequest::parse_headers()
{
	std::string headers_str;

	size_t double_crlf = buffer.find("\r\n\r\n", buffer_pos);
	if (double_crlf == std::string::npos)
	{
		// Check if we've accumulated too much data without finding header end
		const size_t MAX_HEADER_SIZE = 8192; // 8KB for headers
		if (buffer.length() > MAX_HEADER_SIZE)
		{
			connection_status = 0;
			error_code = 431;
			error_message = "Request Header Fields Too Large: Headers exceed 8KB limit";
			current_state = PARSE_ERROR;
			access_error(error_code, error_message);
			buffer.clear(); // Clear buffer to prevent memory leak
			return false;
		}
		return false;
	}
	headers_str = buffer.substr(buffer_pos, double_crlf - buffer_pos);
	buffer_pos = double_crlf + 4;

	std::map<std::string, std::string> header_map;
	std::istringstream stream(headers_str);
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
			error_code = 400;
			error_message = "Bad Request: Invalid header format - no colon found in line: '" + line + "'";
			current_state = PARSE_ERROR;
			access_error(error_code, error_message);
			return false;
		}

		std::string key = line.substr(0, colon_pos);
		std::string value = line.substr(colon_pos + 1);

		if (!key.empty() && (key[key.length() - 1] == ' ' || key[key.length() - 1] == '\t'))
		{
			connection_status = 0;
			error_code = 400;
			error_message = "Bad Request: Invalid header name - trailing whitespace not allowed: '" + key + "'";
			current_state = PARSE_ERROR;
			access_error(error_code, error_message);
			return false;
		}
		if (!key.empty() && (key[0] == ' ' || key[0] == '\t'))
		{
			connection_status = 0;
			error_code = 400;
			error_message = "Bad Request: Invalid header name - leading whitespace not allowed: '" + key + "'";
			current_state = PARSE_ERROR;
			access_error(error_code, error_message);
			return false;
		}

		// Validate header name characters (must be tokens per RFC 7230)
		for (size_t i = 0; i < key.length(); ++i)
		{
			char c = key[i];
			if (c < 33 || c > 126 || c == '(' || c == ')' || c == '<' || c == '>' ||
				c == '@' || c == ',' || c == ';' || c == ':' || c == '\\' ||
				c == '"' || c == '/' || c == '[' || c == ']' || c == '?' ||
				c == '=' || c == '{' || c == '}')
			{
				current_state = PARSE_ERROR;
				connection_status = 0;
				error_code = 400;
				error_message = "Bad Request: Invalid character in header name: '" + std::string(1, c) + "'";
				access_error(error_code, error_message);
				return false;
			}
		}
		if (key.empty())
		{
			connection_status = 0;
			error_code = 400;
			error_message = "Bad Request: Empty header name not allowed";
			current_state = PARSE_ERROR;
			access_error(error_code, error_message);
			return false;
		}

		value.erase(0, value.find_first_not_of(" \t"));
		value.erase(value.find_last_not_of(" \t") + 1);
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		std::transform(value.begin(), value.end(), value.begin(), ::tolower);
		if(key == "host" || key == "transfer-encoding" || key == "content-length" || key == "connection" || key == "user-agent" || key == "content-type" || key == "cookie")
			header_map[key] = value;
	}

	headers = header_map;


	if (!checkHost(headers) || !checkConnection(headers) || !checkTransferEncoding(headers) || !checkContentLength(headers) || !checkLocation(headers) || !checkContentType(headers) || !checkCookie(headers))
	{
		current_state = PARSE_ERROR;
		return false;
	}
	if(content_lenght_exists == 1 && transfer_encoding_exists == 1)
	{
		error_code = 400;
		error_message = "Bad Request: Content-Length and Transfer-Encoding headers cannot be used together";
		access_error(error_code, error_message);
		current_state = PARSE_ERROR;
		return false;
	}
	if (content_lenght_exists == 1)
	{
		std::string content_length_str = headers.at("content-length");
		std::istringstream iss(content_length_str);
		iss >> expected_body_length;
	}
	else
		expected_body_length = 0;
	return true;
}

bool ParsingRequest::checkContentType(const std::map<std::string, std::string>& headers)
{
	if (headers.find("content-type") != headers.end())
	{
		std::string content_type = headers.at("content-type");
		if (content_type.empty())
		{
			connection_status = 0;
			current_state = PARSE_ERROR;
			error_code = 400;
			error_message = "Bad Request: Content-Type header cannot be empty";
			access_error(error_code, error_message);
			return false;
		}
		
		size_t semicolon_pos = content_type.find(';');
		std::string content_type_value;
		std::map<std::string, std::string> content_type_directives;
		
		if (semicolon_pos != std::string::npos)
		{
			content_type_value = content_type.substr(0, semicolon_pos);
			std::string directives_part = content_type.substr(semicolon_pos + 1);
			size_t start = 0;
			size_t next_semicolon;
			
			do {
				next_semicolon = directives_part.find(';', start);
				std::string directive;
				
				if (next_semicolon != std::string::npos)
				{
					directive = directives_part.substr(start, next_semicolon - start);
					start = next_semicolon + 1;
				}
				else
				{
					directive = directives_part.substr(start);
				}
				directive.erase(0, directive.find_first_not_of(" \t"));
				directive.erase(directive.find_last_not_of(" \t") + 1);
				size_t equals_pos = directive.find('=');
				if (equals_pos != std::string::npos)
				{
					std::string key = directive.substr(0, equals_pos);
					std::string value = directive.substr(equals_pos + 1);
					key.erase(0, key.find_first_not_of(" \t"));
					key.erase(key.find_last_not_of(" \t") + 1);
					value.erase(0, value.find_first_not_of(" \t"));
					value.erase(value.find_last_not_of(" \t") + 1);
					std::transform(key.begin(), key.end(), key.begin(), ::tolower);
					std::transform(value.begin(), value.end(), value.begin(), ::tolower);
					content_type_directives[key] = value;
					this->headers[key] = value;
				}
			} while (next_semicolon != std::string::npos);
			if (content_type_directives.find("charset") != content_type_directives.end())
			{
				std::string charset = content_type_directives["charset"];
				if (charset != "utf-8" && charset != "iso-8859-1")
				{
					connection_status = 0;
					error_code = 415;
					error_message = "Unsupported Media Type: Charset '" + charset + "' is not supported";
					current_state = PARSE_ERROR;
					access_error(error_code, error_message);
					return false;
				}
			}
		}
		else
		{
			content_type_value = content_type;
		}
		content_type_value.erase(0, content_type_value.find_first_not_of(" \t"));
		content_type_value.erase(content_type_value.find_last_not_of(" \t") + 1);
		std::transform(content_type_value.begin(), content_type_value.end(), content_type_value.begin(), ::tolower);
		this->headers["content-type-value"] = content_type_value;

		if (content_type_value.find("multipart/") == 0)
		{
			if (content_type_directives.find("boundary") == content_type_directives.end())
			{
				connection_status = 0;
				error_code = 400;
				error_message = "Bad Request: Boundary parameter is required for multipart content types";
				current_state = PARSE_ERROR;
				// access_error(error_code, error_message);
				return false;
			}
			std::string boundary = content_type_directives["boundary"];
			if (boundary.empty())
			{
				connection_status = 0;
				error_code = 400;
				error_message = "Bad Request: Boundary parameter cannot be empty for multipart content types";
				current_state = PARSE_ERROR;
					// access_error(error_code, error_message);
				return false;
			}
		}
		if (content_type_value != "text/html" && 
			content_type_value != "text/plain" && 
			content_type_value != "application/x-www-form-urlencoded" &&
			content_type_value != "multipart/form-data" &&
			content_type_value != "application/json" &&
			content_type_value != "image/png" && 
			content_type_value != "video/mp4" &&
			content_type_value != "image/jpeg" &&
			content_type_value != "image/gif" &&
			content_type_value != "application/xml" &&
			content_type_value != "application/pdf" &&
			content_type_value != "application/octet-stream"	
		)
		{
			connection_status = 0;
			error_code = 415;
			error_message = "Unsupported Media Type: Content-Type '" + content_type_value + "' is not supported";
			current_state = PARSE_ERROR;
			// access_error(error_code, error_message);
			return false;
		}
	}
	return true;
}



bool ParsingRequest::checkConnection(const std::map<std::string, std::string>& headers)
{
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
			error_code = 400;
			error_message = "Bad Request: Invalid Connection header value - must be 'keep-alive' or 'close'";
			current_state = PARSE_ERROR;
			access_error(error_code, error_message);
			return false;
		}
	}
	connection_status = 1;
	return true;
}

bool ParsingRequest::checkContentLength(const std::map<std::string, std::string>& headers)
{
	if (headers.find("content-length") != headers.end())
	{
		content_lenght_exists = 1;
		std::string content_length_str = headers.at("content-length");
		if (content_length_str.empty())
		{
			connection_status = 0;
			error_code = 400;
			error_message = "Bad Request: Content-Length header cannot be empty";
			current_state = PARSE_ERROR;
			access_error(error_code, error_message);
			return false;
		}
		for (size_t i = 0; i < content_length_str.length(); ++i)
		{
			if (!isdigit(content_length_str[i]))
			{
				connection_status = 0;
				error_code = 400;
				error_message = "Bad Request: Content-Length header must be a valid integer - got: '" + content_length_str + "'";
				current_state = PARSE_ERROR;
				access_error(error_code, error_message);
				return false;
			}
		}
		int content_length;
		std::istringstream iss(content_length_str);
		iss >> content_length;
		if (iss.fail() || !iss.eof())
		{
			connection_status = 0;
			error_code = 400;
			error_message = "Bad Request: Content-Length header must be a valid integer - got: '" + content_length_str + "'";
			current_state = PARSE_ERROR;
			access_error(error_code, error_message);
			return false;
		}
		if (content_length < 0)
		{
			connection_status = 0;
			error_code = 400;
			error_message = "Bad Request: Content-Length header cannot be negative - got: '" + content_length_str + "'";
			current_state = PARSE_ERROR;
			access_error(error_code, error_message);
			return false;
		}
		// if(transfer_encoding_exists == 0)
		// {
		// 	if (content_length > 8000)
		// 	{
		// 		connection_status = 0;
		// 		error_code = 413;
		// 		error_message = "Content Too Large: Content-Length exceeds maximum allowed size (8000 bytes) - got: '" + content_length_str + "'";
		// 		current_state = PARSE_ERROR;
		// 		access_error(error_code, error_message);
		// 		return false;
		// 	}
		// }
		// return true;
		// cout << RED "Content length val : " << 
	}
	// content_lenght_exists = 0;
	return true;
}

// i guess its need mor edge cases to handle
//too look after
bool ParsingRequest::checkHost(const std::map<std::string, std::string>& headers)
{
	if (headers.find("host") != headers.end())
	{
		host_exists = 1;
		return true;
	}
	host_exists = 0;
	connection_status = 0;
	error_code = 400;
	current_state = PARSE_ERROR;
	error_message = "Bad Request: Host header is missing";
	access_error(error_code, error_message);
	return false;
}

bool ParsingRequest::checkTransferEncoding(const std::map<std::string, std::string>& headers)
{
	if (headers.find("transfer-encoding") != headers.end())
	{
		// cout << "***********\n";
		// write(1, buffer.data(), buffer.length());
		// cout << "***BUFFER_END***\n";
		std::string transfer_encoding_value = headers.at("transfer-encoding");
		transfer_encoding_exists = 1;
		if (transfer_encoding_value == "gzip" || transfer_encoding_value == "compress" || transfer_encoding_value == "deflate" || transfer_encoding_value == "identity")
		{
			error_code = 501;
			current_state = PARSE_ERROR;
			result_p = PARSE_ERROR_501;
			error_message = "Not Implemented: Transfer-Encoding '" + transfer_encoding_value + "' is not implemented by our webserver :(";
			access_error(error_code, error_message);
			return false;
		}
		else if (transfer_encoding_value != "chunked")
		{
			connection_status = 0;
			error_code = 400;
			error_message = "Bad Request: Invalid Transfer-Encoding header value";
			current_state = PARSE_ERROR;
			access_error(error_code, error_message);
			return false;
		}
	}
	else
	{
		transfer_encoding_exists = 0;
		return true;
	}
	return true;
}

bool ParsingRequest::parse_body()
{
    std::string method = start_line.at("method");

    if (method == "GET" || method == "HEAD" || method == "DELETE") {
        return true;
    }

    if (method == "POST") 
	{
        if (transfer_encoding_exists == 1) 
		{   
            // Check if we have new data since last call
            if (buffer.length() <= chunked_last_processed_size) {
                std::string dummy;
                bool is_complete = refactor_data(dummy, NULL, (size_t)-1);
                if (is_complete) {
                    std::cout << GREEN "CHUNKED - Transfer complete! Final accumulated size: " RESET << chunked_accumulated_data.length() << std::endl;
                    body_content = chunked_accumulated_data;
                    buffer_pos = buffer.length();
                    chunked_last_processed_size = 0;
                    chunked_accumulated_data.clear();
                    return true;
                }                
                return false;
            }
            size_t new_data_start = chunked_last_processed_size;
            size_t new_data_size = buffer.length() - new_data_start;
            const char* new_data = buffer.c_str() + new_data_start;
            if (refactor_data(chunked_accumulated_data, new_data, new_data_size)) {
                body_content = chunked_accumulated_data;
                std::cout << "CHUNKED - Transfer completed! Final body size: " << body_content.length() << std::endl;
                buffer_pos = buffer.length();
                chunked_last_processed_size = 0; // Reset for next request
                chunked_accumulated_data.clear(); // Reset for next request
                return true;
            }
            else {
                chunked_last_processed_size = buffer.length();
                return false;
            }
        }
		else if (content_lenght_exists == 1) 
		{
            size_t available = buffer.length() - buffer_pos;
            if (available < expected_body_length) {
                return false;
            }
            
            body_content = buffer.substr(buffer_pos, expected_body_length);
            buffer_pos += expected_body_length;
            return true;
        }
    }
    return true;
}


// Feed data to the parser 
ParsingRequest::ParseResult ParsingRequest::feed_data(const char* data, size_t len)
{
	const size_t MAX_BUFFER_SIZE = 10 * 1024 * 1024;
	if (buffer.length() + len > MAX_BUFFER_SIZE)
	{
		error_code = 413;
		error_message = "Payload Too Large: Request exceeds maximum allowed size";
		current_state = PARSE_ERROR;
		access_error(error_code, error_message);
		buffer.clear();
		return PARSE_ERROR_RESULT;
	}

	try{
		buffer.append(data, len );
	}
	catch(std::exception& e)
	{
		error_code = 500;
		error_message = "Internal Server Error: Memory allocation failed while appending data to buffer";
		current_state = PARSE_ERROR;
		access_error(error_code, error_message);
		buffer.clear(); // Clear buffer to prevent memory leak
		return PARSE_ERROR_RESULT;
	}
	while (current_state != PARSE_COMPLETE && current_state != PARSE_ERROR)
	{
		
		switch (current_state)
		{
		case PARSE_START_LINE:
			if (!parse_start_line())
			{
				if (current_state == PARSE_ERROR)
					break;
				return PARSE_AGAIN;
			}
			current_state = PARSE_HEADERS;
			break;

		case PARSE_HEADERS:
			if (!parse_headers())
			{
				if (current_state == PARSE_ERROR)
					break;
				return PARSE_AGAIN;
			}
			{
				std::string method = start_line.at("method");
				if (method == "POST" || expected_body_length > 0)
					current_state = PARSE_BODY;
				else
					current_state = PARSE_COMPLETE;
			}
			break;

		case PARSE_BODY:
			if (!parse_body())
			{
				if (current_state == PARSE_ERROR)
					break;
				return PARSE_AGAIN;
			}
			current_state = PARSE_COMPLETE;
			break;

		default:
			break;
		}
	}
	if (current_state == PARSE_ERROR)
	{
		return PARSE_ERROR_RESULT;
	}
	return PARSE_OK;
}




//Host
//Content Length
//Tranfer encoding 
// Connection
//Content Type // respond
// expect
//User agent
// Authorization
// cookies

// | Header | Required | Purpose                            |
// | ------------------ - | -------------------------- | ---------------------------------- |
// | `Host` | ✅ Yes | Domain name of the request         |
// | `Content-Length` | ✅ If body | Length of the request body         |
// | `Transfer-Encoding` | ❌ Skip or reject `chunked` | Alternate body encoding            |
// | `Connection` | Optional | Controls connection persistence    |
// | `Content-Type` | ✅ If body | MIME type of the body              |
// | `Expect` | Optional | Expect 100 - continue                |
// | `User-Agent` | Optional | Client info                        |
// | `Authorization` | Optional | Used for auth                      |
// | `Location` | Optional | Redirect URL for 3xx responses     |
// | `Cookies` | Optional | Client cookies for session state   |
// | Others | ❌ Ignore | Not needed for basic functionality |

void ParsingRequest::reset()
{
    // If there's leftover data in buffer after current request, preserve it
    std::string leftover_data;
    if (buffer_pos < buffer.length()) {
        leftover_data = buffer.substr(buffer_pos);
    }
    
    // Reset all state variables for a new request
    start_line.clear();
    headers.clear();
    body_content.clear();
    buffer.clear();
    buffer_pos = 0;
    expected_body_length = 0;
    chunked_last_processed_size = 0;
    chunked_accumulated_data.clear();
    current_state = PARSE_START_LINE;
    connection_status = 1;
    content_lenght_exists = 0;
    transfer_encoding_exists = 0;
    host_exists = 0;
    error_code = 0;
    error_message.clear();
    status_code = 200;
    status_phrase.clear();
    
    // Restore any leftover data that might belong to the next request
    if (!leftover_data.empty()) {
        buffer = leftover_data;
    }
    
    // Reset the static state in refactor_data function
    reset_refactor_data_state();
}

bool ParsingRequest::checkCookie(const std::map<std::string, std::string>& headers)
{
	if (headers.find("cookie") != headers.end())
	{
		std::string cookie_string = headers.at("cookie");
		// std::cout<<"Cookie string: " << cookie_string << std::endl;
		if (cookie_string.empty())
			return true;
		this->headers["cookie"] = cookie_string;
	}
	return true;
}

std::string ParsingRequest::getId() const
{
	std::string cookies = getCookies();
	if (cookies.empty())
		return "";
	
	size_t pos = cookies.find("id=");
	if (pos == std::string::npos)
		return "";
	pos += 3; 
	size_t end_pos = cookies.find(';', pos);
	if (end_pos == std::string::npos)
		end_pos = cookies.length();
	
	return cookies.substr(pos, end_pos - pos);
}

std::string ParsingRequest::generateSetCookieHeader(const std::string& name, const std::string& value) const
{
	return "Set-Cookie: " + name + "=" + value + "; Path=/; HttpOnly\r\n";
}

std::string ParsingRequest::getCookies() const
{
	if (headers.find("cookie") != headers.end()) {
		return headers.at("cookie");
	}
	return "";
}
