#include "../../inc/Get.hpp"
#include "../../inc/POST.hpp"

string	getClosest(vector<string> locationsName, string uri)
{
	int		bestIndex;
	size_t	bestLength;
	string	loc;

	bestIndex = -1;
	bestLength = 0;
	for (size_t i = 0; i < locationsName.size(); ++i)
	{
		loc = locationsName[i];
		if (uri.compare(0, loc.length(), loc) == 0)
		{
			if (uri.length() == loc.length() || uri[loc.length()] == '/'
				|| loc[loc.length() - 1] == '/')
			{
				if (loc.length() > bestLength)
				{
					bestIndex = i;             // 0
					bestLength = loc.length(); // 1
				}
			}
		}
	}
	if (bestIndex == -1)
		return ("No matching loc found!");
	else
		return (locationsName.at(bestIndex));
}
// I"LL RETURN THE LOCATION STRUCT ONLY LATER
// std::pair<std::string,LocationStruct> get_location(ConfigStruct conf,
// string uri)
// {
//     vector<string> locations;

//     // for(auto it = conf._cluster.begin(); it != conf._cluster.end(); it++)
//     // {
//
//     for(auto i = (*it).second.location.begin(); i != (*it).second.location.end(); i++)
//     //         locations.push_back((*i).first);
//     // }

//     string matchedLoc = getClosest(locations, uri);

//     cout << matchedLoc << endl;

//     //CHECK IT LATER SINCE YOU RETURNING THE LOCATION NOW!!!!!!
//     // if(matchedLoc == "No matching loc found!")
//     // {
//     //     return (Error::notFound());
//     // }

//     //GET THE WANTED LOCATION

//     std::pair<std::string,LocationStruct> location;

//     // for(auto it = conf._cluster.begin(); it != conf._cluster.end(); it++)
//     // {
//
//     for(auto i = (*it).second.location.begin(); i != (*it).second.location.end(); i++)
//     //     {
//     //         if((*i).first == matchedLoc)
//     //         {
//     //             location = (*i);
//     //             break ;
//     //         }
//     //     }
//     // }
//     return (location);
// }

string	handle_redirect(std::pair<std::string, LocationStruct> location)
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
	string response = "HTTP/1.1 " + status + " " + statusMsg +
		"\r\n"
		"Location: " +
		newLoc +
		"\r\n"
		"Content-Length: 0\r\n"
		"\r\n";
	return response;
	// send(fd, response.c_str(), response.length(), 0);
}

string	handle_notAllowed(std::pair<std::string,
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
	// send(fd, res.c_str(), res.length(), 0);
}

string	notFound()
{
	string response = "HTTP/1.1 404 Not Found\r\n"
						"Content-Type: text/html\r\n"
						"Content-Length: 48\r\n"
						"\r\n"
						"<html><body><h1>404 Not Found</h1></body></html>";
	return response;
	// send(fd, response.c_str(), response.length(), 0);
}

// std::pair<std::string, LocationStruct> get_location(int fd,
// 	const std::string &requestPath, const ConfigStruct &server)
// {
// 	size_t	lastSlash;

// 	std::string path = requestPath;
// 	std::string removedSegment;
// 	std::string removedPath;
// 	while (true)
// 	{
// 	for (size_t i = 0; i < server.location.size(); ++i)
// 	{
// 		if (path == server.location[i].first)
// 		{
// 			cout << server.location[i].first << endl;
// 			// if(server.location[i].second._return.)
// 			if (server.location[i].second._return.size() != 0)
// 			{
// 				handle_redirect(fd, server.location[i]);
// 				// I need to return to the main loop after that??
// 				throw runtime_error("Redirection");
// 			}
// 			if (server.location[i].second.allowedMethods.find("POST") == server.location[i].second.allowedMethods.end())
// 			{
// 				handle_notAllowed(fd, server.location[i]);
// 				throw runtime_error("Method not allowed");
// 			}
// 			// this->_name_location = server.location[i].first;
// 			// cout << server.location[i].second.root + removedPath << endl;
// 			// return (server.location[i].second.root + removedPath);
// 			return (server.location[i]);
// 		}
// 	}
// 	if (path == "/")
// 	break ;
// 	lastSlash = path.find_last_of('/');
// 	if (lastSlash == std::string::npos || lastSlash == 0)
// 	{
// 		removedSegment = path.substr(lastSlash);
// 		removedPath = removedSegment + removedPath;
// 		path = "/";
// 	}
// 	else
// 	{
// 		removedSegment = path.substr(lastSlash);
// 		removedPath = removedSegment + removedPath;
// 		path = path.substr(0, lastSlash);
// 	}
// 	notFound(fd);
// 	throw runtime_error("Location not found");
// }
// 	// return ("");
// }



void	forbidden_error(int fd)
{
	string response = "HTTP/1.1 403 Forbidden\r\n"
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
	send(fd, response.c_str(), response.length(), 0);
}

// vector<pair<string, string>> get_first_line(string body)
// {

// }



void	testFun(string boundary, string body)
{
	size_t	pos;
	size_t	headerEnd;
	size_t	contentStart;
	size_t	nextBoundary;
	std::string headers;
	std::string content;
	
	pos = 0;
	while ((pos = body.find(boundary, pos)) != std::string::npos)
	{
		pos += boundary.size();
		// Skip leading CRLF
		if (body.compare(pos, 2, "\r\n") == 0)
			pos += 2;
		// Find end of headers
		headerEnd = body.find("\r\n\r\n", pos);
		if (headerEnd == std::string::npos)
			break ;
		headers = body.substr(pos, headerEnd - pos);
		// Content starts after headerEnd+4
		contentStart = headerEnd + 4;
		// Find next boundary
		nextBoundary = body.find(boundary, contentStart);
		content = body.substr(contentStart, nextBoundary
				- contentStart);
				pos = nextBoundary;
	}
	std::cout << "Headers:\n" << headers << "\n";
	std::cout << "Content:\n" << content << "\n";
}

void	parse_body(ParsingRequest parser, int fd, LocationStruct location)
{
	testFun(parser.getHeaders()["boundary"], parser.getBody());
	cout << fd << endl;
	cout << location.root << endl;
}

void	handle_upload(int fd, ConfigStruct config, LocationStruct location,
		ParsingRequest parser)
{
	string	upload_path;
	cout << config.root << endl;
	// cout << "-------------------> " << location.upload_enabled << endl;
	if (location.upload_enabled == false)
	{
		forbidden_error(fd);
		return ;
	}
	if (location.upload_path.empty())
		upload_path = location.root;
	else
		upload_path = location.upload_path;
	parse_body(parser, fd, location);
}

// void	postMethod(int fd, string uri, ConfigStruct config,
// 		ParsingRequest &parser)
// {
// 	try
// 	{
// 		std::pair<std::string, LocationStruct> location = get_location(fd, uri,
// 				config);

// 		handle_upload(fd, config, location.second, parser);

// 		// cout << location.first << endl;
// 	}
// 	catch (exception &e)
// 	{
// 	}
// }