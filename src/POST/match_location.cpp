#include "../../inc/webserv.hpp"

std::pair<std::string, LocationStruct> get_location(const std::string &requestPath, const ConfigStruct &server)
{
	LocationStruct	bestLocation;
	size_t			bestLength;

	std::string bestMatch;
	bestLength = 0;
	for (size_t i = 0; i < server.location.size(); ++i)
	{
		const std::string &locPath = server.location[i].first;
		if (requestPath.find(locPath) == 0) // prefix match
		{
			if (locPath.size() > bestLength) // longest match wins
			{
				bestMatch = locPath;
				bestLocation = server.location[i].second;
				bestLength = locPath.size();
			}
		}
	}
	if (!bestMatch.empty())
	{
		if (!bestLocation._return.empty())
		{
			// handle_redirect(fd, std::make_pair(bestMatch, bestLocation));
			throw runtime_error("Redirection");
		}
		if (bestLocation.allowedMethods.find("POST") == bestLocation.allowedMethods.end())
		{
			// handle_notAllowed(fd, std::make_pair(bestMatch, bestLocation));
			throw runtime_error("Method not allowed");
		}
		return (std::make_pair(bestMatch, bestLocation));
	}
	// notFound(fd);
	throw runtime_error("Location not found");
}