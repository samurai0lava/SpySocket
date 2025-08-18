#include "../../inc/webserv.hpp"


// NGINX-style incremental parsing implementation (like wtfffff)

bool ParsingRequest::checkURI(const std::string& uri)
{
	if (uri.empty())
	{
		connection_status = 0;
		error_code = 400;
		error_message = "Bad Request: URI cannot be empty";
		logError(error_code, error_message);
		current_state = PARSE_ERROR;
		return false;
	}
	if (uri[0] != '/')
	{
		connection_status = 0;
		error_code = 400;
		error_message = "Bad Request: URI must start with '/' - got: '" + uri + "'";
		logError(error_code, error_message);
		current_state = PARSE_ERROR;
		return false;
	}
	if (uri.length() > 8000)
	{
		connection_status = 0;
		error_code = 414;
		error_message = "URI Too Long: URI exceeds 8000 characters";
		logError(error_code, error_message);
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
			logError(error_code, error_message);
			current_state = PARSE_ERROR;
			return false;
		}
		if (c == ' ')
		{
			connection_status = 0;
			error_code = 400;
			error_message = "Bad Request: URI contains unencoded spaces";
			current_state = PARSE_ERROR;
			logError(error_code, error_message);
			return false;
		}
	}

	return true;
}
bool ParsingRequest::checkVersion(const std::string& version)
{
	if (version.empty())
	{
		connection_status = 0;
		error_code = 400;
		error_message = "Bad Request: HTTP version cannot be empty";
		logError(error_code, error_message);
		current_state = PARSE_ERROR;
		return false;
	}
	if (version != "HTTP/1.0" && version != "HTTP/1.1")
	{
		connection_status = 0;
		error_code = 400;
		error_message = "Bad Request: Invalid HTTP version: '" + version + "'";
		current_state = PARSE_ERROR;
		logError(error_code, error_message);
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
		logError(error_code, error_message);
		return false;
	}

	if (method == "PUT" || method == "PATCH" || method == "OPTIONS" || method == "TRACE" || method == "CONNECT")
	{
		error_code = 501;
		error_message = "Not Implemented: HTTP method '" + method + "' is not implemented";
		current_state = PARSE_ERROR;
		result_p = PARSE_ERROR_501;

		logError(error_code, error_message);
		return false;
	}
	else if (method != "GET" && method != "POST" && method != "DELETE" && method != "HEAD")
	{
		connection_status = 0;
		error_code = 400;
		error_message = "Bad Request: Invalid HTTP method: '" + method + "'";
		current_state = PARSE_ERROR;
		logError(error_code, error_message);
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

	// Parse start line directly
	std::istringstream ss(start_line_str);
	std::string method, uri, version;

	ss >> method >> uri >> version;

	start_line["method"] = method;
	start_line["uri"] = uri;
	start_line["version"] = version;

	// Validate method directly
	if (!checkMethod(method) || !checkURI(uri) || !checkVersion(version))
		return false;

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
			logError(error_code, error_message);
			return false;
		}
		if (location[0] != '/')
		{
			connection_status = 0;
			error_code = 400;
			error_message = "Bad Request: Location header must start with '/' - got: '" + location + "'";
			current_state = PARSE_ERROR;
			logError(error_code, error_message);
			return false;
		}
		if (location.length() > 8000)
		{
			connection_status = 0;
			error_code = 400;
			error_message = "Bad Request: Location header too long (exceeds 8000 characters)";
			current_state = PARSE_ERROR;
			logError(error_code, error_message);
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
				logError(error_code, error_message);
				return false;
			}
			if (c == ' ')
			{
				connection_status = 0;
				error_code = 400;
				error_message = "Bad Request: Location header contains unencoded spaces";
				current_state = PARSE_ERROR;
				logError(error_code, error_message);
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
		return false;
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
			logError(error_code, error_message);
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
			logError(error_code, error_message);
			return false;
		}
		if (!key.empty() && (key[0] == ' ' || key[0] == '\t'))
		{
			connection_status = 0;
			error_code = 400;
			error_message = "Bad Request: Invalid header name - leading whitespace not allowed: '" + key + "'";
			current_state = PARSE_ERROR;
			logError(error_code, error_message);
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
				logError(error_code, error_message);
				return false;
			}
		}
		if (key.empty())
		{
			connection_status = 0;
			error_code = 400;
			error_message = "Bad Request: Empty header name not allowed";
			current_state = PARSE_ERROR;
			logError(error_code, error_message);
			return false;
		}

		value.erase(0, value.find_first_not_of(" \t"));
		value.erase(value.find_last_not_of(" \t") + 1);
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		std::transform(value.begin(), value.end(), value.begin(), ::tolower);
		if(key == "content-length" || key == "transfer-encoding" || key == "host" || key == "connection" || key == "user-agent" || key == "content-type")
			header_map[key] = value;
	}

	headers = header_map;


	if (!checkHost(headers) || !checkConnection(headers) || !checkContentLength(headers) || !checkTransferEncoding(headers) || !checkLocation(headers) || !checkContentType(headers))
	{
		current_state = PARSE_ERROR;
		return false;
	}

	// Check content length for body parsing
	if (content_lenght_exists)
	{
		std::string content_length_str = headers.at("content-length");
		std::istringstream iss(content_length_str);
		iss >> expected_body_length;
	}

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
			logError(error_code, error_message);
			return false;
		}
		if (content_type.find("text/") == 0 || content_type.find("application/") == 0 || content_type.find("image/") == 0)
		{
			return true;
		}
		else
		{
			connection_status = 0;
			error_code = 415;
			error_message = "Unsupported Media Type: Content-Type '" + content_type + "' is not supported";
			current_state = PARSE_ERROR;
			logError(error_code, error_message);
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
			logError(error_code, error_message);
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
			logError(error_code, error_message);
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
				logError(error_code, error_message);
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
			logError(error_code, error_message);
			return false;
		}
		if (content_length < 0)
		{
			connection_status = 0;
			error_code = 400;
			error_message = "Bad Request: Content-Length header cannot be negative - got: '" + content_length_str + "'";
			current_state = PARSE_ERROR;
			logError(error_code, error_message);
			return false;
		}
		if (content_length > 8000)
		{
			connection_status = 0;
			error_code = 413;
			error_message = "Content Too Large: Content-Length exceeds maximum allowed size (8000 bytes) - got: '" + content_length_str + "'";
			current_state = PARSE_ERROR;
			logError(error_code, error_message);
			return false;
		}
		return true;
	}
	content_lenght_exists = 0;
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
	logError(error_code, error_message);
	return false;
}

bool ParsingRequest::checkTransferEncoding(const std::map<std::string, std::string>& headers)
{
	if (headers.find("transfer-encoding") != headers.end())
	{
		std::string transfer_encoding_value = headers.at("transfer-encoding");
		transfer_encoding_exists = 1;
		if (transfer_encoding_value == "gzip" || transfer_encoding_value == "compress" || transfer_encoding_value == "deflate" || transfer_encoding_value == "identity")
		{
			error_code = 501;
			current_state = PARSE_ERROR;
			result_p = PARSE_ERROR_501;
			error_message = "Not Implemented: Transfer-Encoding '" + transfer_encoding_value + "' is not implemented by our webserver :(";
			logError(error_code, error_message);
			return false;
		}
		else if (transfer_encoding_value != "chunked")
		{
			connection_status = 0;
			error_code = 400;
			error_message = "Bad Request: Invalid Transfer-Encoding header value - must be 'chunked'";
			current_state = PARSE_ERROR;
			logError(error_code, error_message);
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

//parsing body if available // Cases aaaaaaaaaaaaaaa
bool ParsingRequest::parse_body()
{
	size_t available = buffer.length() - buffer_pos;
	if (available < expected_body_length)
		return false;
	body_content = buffer.substr(buffer_pos, expected_body_length);
	buffer_pos += expected_body_length;

	return true;
}


// Feed data to the parser 
ParsingRequest::ParseResult ParsingRequest::feed_data(const char* data, size_t len)
{

	buffer.append(data, len);
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
			if (expected_body_length > 0)
				current_state = PARSE_BODY;
			else
				current_state = PARSE_COMPLETE;
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
    // Reset all state variables for a new request
    start_line.clear();
    headers.clear();
    body_content.clear();
    buffer.clear();
    buffer_pos = 0;
    expected_body_length = 0;
    current_state = PARSE_START_LINE;
    connection_status = 1;
    content_lenght_exists = 0;
    transfer_encoding_exists = 0;
    host_exists = 0;
    error_code = 0;
    error_message.clear();
    status_code = 200;
    status_phrase.clear();
}
	