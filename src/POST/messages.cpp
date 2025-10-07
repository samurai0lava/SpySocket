#include "../../inc/Get.hpp"
#include "../../inc/POST.hpp"

std::string	handle_redirect(std::pair<std::string, LocationStruct> location)
{
	std::string status = (*location.second._return.begin()).first;
	// Not sure why there is a vector i'll work with the first one for now
	std::string newLoc = (*location.second._return.begin()).second;
	std::string statusMsg;
	if (status == "301")
		statusMsg = "Moved Permanently";
	else if (status == "302")
		statusMsg = "Found";
	else if (status == "307")
		statusMsg = "Temporary Redirect";
	else if (status == "308")
		statusMsg = "Permanent Redirect";
	else
		statusMsg = "Redirect"; // fallback
	std::string response = "HTTP/1.1 " + status + " " + statusMsg +
		"\r\n"
		"Location: " +
		newLoc +
		"\r\n"
		"Content-Length: 0\r\n"
		"\r\n";
	return response;
}

std::string	handle_notAllowed(std::pair<std::string,
	LocationStruct> location)
{
	std::string res = "HTTP/1.1 405 Method Not Allowed\r\nAllow: ";
	for (std::set<std::string>::iterator it = location.second.allowedMethods.begin(); it != location.second.allowedMethods.end(); ++it)
	{
		res += *it;
		if (it++ != location.second.allowedMethods.end())
		{
			res += ", ";
		}
		it--;
	}
	std::string body = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
	res += "\r\nContent-Type: text/html\r\n";
	std::stringstream ss;
	ss << body.size();
	res += "Content-Length: " + ss.str() + "\r\n";
	res += "\r\n";
	res += body;
	return res;
}

std::string notFound()
{
	std::string response = "HTTP/1.1 404 Not Found\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 48\r\n"
		"\r\n"
		"<html><body><h1>404 Not Found</h1></body></html>";
	return response;
}
std::string bad_request()
{
	return "HTTP/1.1 400 Bad Request\r\n"
		"Content-Type: text/html; charset=UTF-8\r\n"
		"Content-Length: 113\r\n"
		"Connection: close\r\n"
		"\r\n"
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head><title>400 Bad Request</title></head>\n"
		"<body><h1>400 Bad Request</h1><p>Your request is invalid.</p></body>\n"
		"</html>\n";
}

std::string forbidden_403()
{
	return "HTTP/1.1 403 Forbidden\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 112\r\n"
		"Connection: close\r\n"
		"\r\n"
		"<html>"
		"<head><title>403 Forbidden</title></head>"
		"<body>"
		"<h1>Forbidden</h1>"
		"<p>You don't have permission to access this resource.</p>"
		"</body>"
		"</html>";
}

std::string internal_error()
{
	return "HTTP/1.1 500 Internal Server Error\r\n"
		"Content-Type: text/html; charset=UTF-8\r\n"
		"Content-Length: 164\r\n"
		"Connection: close\r\n"
		"\r\n"
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head><title>500 Internal Server Error</title></head>\n"
		"<body><h1>500 Internal Server Error</h1><p>Unexpected server error.</p></body>\n"
		"</html>\n";
}

std::string created_success()
{
	return "HTTP/1.1 201 Created\r\n"
		"Content-Type: text/html; charset=UTF-8\r\n"
		"Content-Length: 142\r\n"
		"Connection: keep-alive\r\n"
		"\r\n"
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head><title>201 Created</title></head>\n"
		"<body><h1>201 Created</h1><p>Resource created successfully.</p></body>\n"
		"</html>\n";
}

std::string OK_200(std::string& body)
{
	std::stringstream ss;
	ss << "HTTP/1.1 200 OK\r\n"
		<< "Date: "
		<< ft_time_format()
		<< "\r\n"
		<< "Server: SpySocket/1.0\r\n"
		<< "Content-Type: text/html; charset=UTF-8\r\n"
		<< "Content-Length: " << body.size() << "\r\n"
		<< "Connection: close\r\n"
		<< "\r\n"
		<< body;
	return ss.str();
}