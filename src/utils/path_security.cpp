#include "../../inc/webserv.hpp"

bool is_path_secure(const std::string &fullPath)
{
	size_t pos = 0;
	while ((pos = fullPath.find("..", pos)) != std::string::npos)
	{
		// Check if ".." is a standalone component (not part of a filename like "..file" or "file..")
		bool isStart = (pos == 0);
		bool isPrecededBySlash = (pos > 0 && fullPath[pos - 1] == '/');
		bool isEnd = (pos + 2 == fullPath.length());
		bool isFollowedBySlash = (pos + 2 < fullPath.length() && fullPath[pos + 2] == '/');

		// ".." is dangerous if it's isolated as a path component
		if ((isStart || isPrecededBySlash) && (isEnd || isFollowedBySlash))
		{
			return false; // Path traversal attempt detected
		}
		pos += 2; // Move past the current ".."
	}
	return true; // Path is secure
}
