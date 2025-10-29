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
	std::string new_id = CookieManager::generateSimpleId();
	std::string response = "HTTP/1.1 " + status + " " + statusMsg + "\r\n"
		"Location: " + newLoc + "\r\n"
		"Content-Length: 0\r\n"
		"Date: " + ft_time_format() + "\r\n"
		"Server: SpySocket/1.0\r\n"
		+ CookieManager::generateSetCookieHeader("id", new_id)
		+ "\r\n";
	return response;
}

std::string	handle_notAllowed(std::pair<std::string,
	LocationStruct> location)
{
	std::string new_id = CookieManager::generateSimpleId();
	std::string res = "HTTP/1.1 405 Method Not Allowed\r\n";
	res += "Allow: ";

	for(std::set<std::string>::iterator it = location.second.allowedMethods.begin(); it != location.second.allowedMethods.end();)
	{
		res += *it;
		it++;
		if (it != location.second.allowedMethods.end())
			res += ", ";
	}
	res += "\r\n";

	std::string body = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
	std::stringstream ss;
	ss << body.size();

	res += "Content-Type: text/html\r\n";
	res += "Content-Length: " + ss.str() + "\r\n";
    res += "Date: ";
	res += ft_time_format();
	res += "\r\n";
	res += "Server: SpySocket/1.0\r\n";
	res += CookieManager::generateSetCookieHeader("id", new_id);
	res += "\r\n";


	res += body;
	std::cout << RED << res << RESET << std::endl;
	return res;
}

std::string notFound()
{
	std::string response = "HTTP/1.1 404 Not Found\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 48\r\n";
	std::string new_id = CookieManager::generateSimpleId();
    response += "Date: ";
	response += ft_time_format();
	response += "\r\nServer: SpySocket/1.0\r\n";
	response += CookieManager::generateSetCookieHeader("id", new_id);
	response +=	"\r\n"
		"<html><body><h1>404 Not Found</h1></body></html>";
	return response;
}
std::string bad_request()
{
	std::string new_id = CookieManager::generateSimpleId();

	std::string response = "HTTP/1.1 400 Bad Request\r\n"
		"Content-Type: text/html; charset=UTF-8\r\n"
		"Content-Length: 113\r\n"
		"Connection: close\r\n";
    response += "Date: ";
	response += ft_time_format();
	response += "\r\nServer: SpySocket/1.0\r\n";
	response += CookieManager::generateSetCookieHeader("id", new_id);
	response +=	"\r\n"
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head><title>400 Bad Request</title></head>\n"
		"<body><h1>400 Bad Request</h1><p>Your request is invalid.</p></body>\n"
		"</html>\n";
	return response;
}

std::string forbidden_403()
{
	std::string new_id = CookieManager::generateSimpleId();

	std::string response = "HTTP/1.1 403 Forbidden\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 112\r\n"
		"Connection: close\r\n";
    response += "Date: ";
	response += ft_time_format();
	response += "\r\nServer: SpySocket/1.0\r\n";
	response += CookieManager::generateSetCookieHeader("id", new_id);
	response +=	"\r\n"
		"<html>"
		"<head><title>403 Forbidden</title></head>"
		"<body>"
		"<h1>Forbidden</h1>"
		"<p>You don't have permission to access this resource.</p>"
		"</body>"
		"</html>";
	return response;
}

std::string internal_error()
{
	std::string new_id = CookieManager::generateSimpleId();

	std::string response = "HTTP/1.1 500 Internal Server Error\r\n"
		"Content-Type: text/html; charset=UTF-8\r\n"
		"Content-Length: 164\r\n"
		"Connection: close\r\n";
	response += "Date: ";
	response += ft_time_format();
	response += "\r\nServer: SpySocket/1.0\r\n";
	response += CookieManager::generateSetCookieHeader("id", new_id);
	response +=	"\r\n"
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head><title>500 Internal Server Error</title></head>\n"
		"<body><h1>500 Internal Server Error</h1><p>Unexpected server error.</p></body>\n"
		"</html>\n";
	return response;
}

std::string created_success()
{
	std::string new_id = CookieManager::generateSimpleId();

	std::string response = "HTTP/1.1 201 Created\r\n";
	response +=	"Content-Type: text/html; charset=UTF-8\r\n";
	response +=	"Content-Length: 142\r\n";
	response +=	"Connection: keep-alive\r\n";
	response += "Date: ";
	response += ft_time_format();
	response += "Server: SpySocket/1.0\r\n";
	response += CookieManager::generateSetCookieHeader("id", new_id);
	response += "\r\n";
	response +=	"<!DOCTYPE html>\n";
	response +=	"<html>\n";
	response +=	"<head><title>201 Created</title></head>\n";
	response +=	"<body><h1>201 Created</h1><p>Resource created successfully.</p></body>\n";
	response +=	"</html>\n";
	return response;
}

std::string OK_200(std::string& body)
{
	std::string new_id = CookieManager::generateSimpleId();

	std::stringstream ss;
	ss << "HTTP/1.1 200 OK\r\n"
		<< "Date: "
		<< ft_time_format()
		<< "\r\n"
		<< "Server: SpySocket/1.0\r\n"
		<< "Content-Type: text/html; charset=UTF-8\r\n"
		<< "Content-Length: " << body.size() << "\r\n"
		<< "Connection: close\r\n"
		<< CookieManager::generateSetCookieHeader("id", new_id)
		<< "\r\n"
		<< body;

	return ss.str();
}

// std::string large_payload()
// {
// 	std::string new_id = CookieManager::generateSimpleId();

// 	std::string response =
//     "HTTP/1.1 413 Payload Too Large\r\n"
//     "Content-Type: text/html\r\n"
//     "Connection: close\r\n"
//     "Content-Length: 94\r\n";
// 	response += "Date: ";
// 	response += ft_time_format();
// 	response += "\r\nServer: SpySocket/1.0\r\n";
// 	response += CookieManager::generateSetCookieHeader("id", new_id);
//     response += "\r\n";
//     response +="<html><body><h1>413 Payload Too Large</h1><p>The request body is too large.</p></body></html>";
// 	return response;
// }


std::string getStatusMessage(int statusCode)
{
    switch(statusCode)
    {
		//No redirection mentionned?
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 413: return "Content Too Large";
        case 414: return "URI Too Long";
        case 415: return "Unsupported Media Type";
        case 429: return "Too Many Requests";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Timeout";
        case 505: return "HTTP Version Not Supported";
        default: return "Unknown Error";
    }
}


std::string getErrorPageFromConfig(int statusCode, ConfigStruct& config)
{
    for (size_t i = 0; i < config.errorPage.size(); ++i)
    {
        if (std::atoi(config.errorPage[i].first.c_str()) == statusCode)
        {
            std::string root = config.root;
            std::string errorPage = config.errorPage[i].second;
            if (!root.empty() && root[root.length() - 1] != '/') {
                root += "/";
            }
            if (!errorPage.empty() && errorPage[0] == '/') {
                errorPage = errorPage.substr(1);
            }
            std::string errorPagePath = root + errorPage;
            struct stat fileStat;
            if (stat(errorPagePath.c_str(), &fileStat) != 0) {
                continue;
            }
            if (!S_ISREG(fileStat.st_mode)) {
                continue;
            }

            std::ifstream file(errorPagePath.c_str(), std::ios::in | std::ios::binary);
            if (file.is_open())
            {
                std::stringstream buffer;
                buffer << file.rdbuf();
                file.close();
                std::ostringstream response;
                response << "HTTP/1.1 " << statusCode << " " << getStatusMessage(statusCode) << "\r\n";
                response << "Content-Type: text/html\r\n";
                response << "Content-Length: " << buffer.str().size() << "\r\n\r\n";
                response << buffer.str();
                return response.str();
            }
        }
    }
    return GenerateResErr(statusCode);
}
