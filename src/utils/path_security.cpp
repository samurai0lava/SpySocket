#include "../../inc/webserv.hpp"

bool is_path_secure(const std::string &fullPath)
{
	// Empty path is safe (represents root directory)
	if (fullPath.empty())
		return true;

	// Path starting with .. is dangerous
	if (fullPath.find("..") == 0)
		return false;

	size_t pos = 0;
	while ((pos = fullPath.find("..", pos)) != std::string::npos)
	{
		bool validPrefix = (pos == 0 || fullPath[pos - 1] == '/');
		bool validSuffix = (pos + 2 >= fullPath.length() || fullPath[pos + 2] == '/');

		if (validPrefix && validSuffix)
			return false;
		pos += 2;
	}

	if (fullPath.find("%2e%2e") != std::string::npos ||
		fullPath.find("%2E%2E") != std::string::npos ||
		fullPath.find("..%2f") != std::string::npos ||
		fullPath.find("..%5c") != std::string::npos)
		return false;

	if (fullPath.find("//") != std::string::npos)
		return false;

	if (!fullPath.empty() && fullPath[0] == '/')
		return false;
	return true;
}
