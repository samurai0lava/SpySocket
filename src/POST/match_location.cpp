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
		if (requestPath.find(locPath) == 0)
		{
			if (locPath.size() > bestLength)
			{
				bestMatch = locPath;
				bestLocation = server.location[i].second;
				bestLength = locPath.size();
			}
		}
	}
	if (!bestMatch.empty())
	{
		return (std::make_pair(bestMatch, bestLocation));
	}
	return (std::make_pair("", bestLocation));
}
