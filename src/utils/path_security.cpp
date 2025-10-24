#include "../../inc/webserv.hpp"
#include <limits.h>
#include <stdlib.h>

/**
 * @brief Validates that the resolved path stays within the allowed root directory
 *
 * This function protects against:
 * - Path traversal attacks (../)
 * - Symlink attacks (following symlinks outside web root)
 * - Absolute path injections
 *
 * @param path The file path to validate (can contain symlinks)
 * @param root The root directory that path must stay within
 * @return true if path is safe and within root, false otherwise
 */
bool isPathSafe(const std::string& path, const std::string& root)
{
	char resolvedPath[PATH_MAX];
	char resolvedRoot[PATH_MAX];

	// Resolve the actual filesystem path (follows symlinks)
	char* realPath = realpath(path.c_str(), resolvedPath);
	if (realPath == NULL)
	{
		// Path doesn't exist yet (could be for upload), check parent directory
		std::string parentPath = path;
		size_t lastSlash = parentPath.find_last_of('/');

		if (lastSlash != std::string::npos)
		{
			parentPath = parentPath.substr(0, lastSlash);
			if (parentPath.empty())
				parentPath = "/";

			realPath = realpath(parentPath.c_str(), resolvedPath);
			if (realPath == NULL)
			{
				// Parent directory doesn't exist either
				return false;
			}

			// Reconstruct the full path with the filename
			std::string filename = path.substr(lastSlash);
			std::string checkPath = std::string(resolvedPath) + filename;
			strncpy(resolvedPath, checkPath.c_str(), PATH_MAX - 1);
			resolvedPath[PATH_MAX - 1] = '\0';
		}
		else
		{
			return false;
		}
	}

	// Resolve the root directory path
	char* realRoot = realpath(root.c_str(), resolvedRoot);
	if (realRoot == NULL)
	{
		std::cerr << "Error: Cannot resolve root directory: " << root << std::endl;
		return false;
	}

	// Ensure root path ends without trailing slash for comparison
	size_t rootLen = strlen(resolvedRoot);
	if (rootLen > 1 && resolvedRoot[rootLen - 1] == '/')
	{
		resolvedRoot[rootLen - 1] = '\0';
		rootLen--;
	}

	// Check if resolved path starts with root path
	size_t pathLen = strlen(resolvedPath);

	// Path must be at least as long as root
	if (pathLen < rootLen)
		return false;

	// Check if path starts with root
	if (strncmp(resolvedPath, resolvedRoot, rootLen) != 0)
		return false;

	// If path is longer than root, next character must be '/' or end of string
	if (pathLen > rootLen && resolvedPath[rootLen] != '/')
		return false;

	return true;
}

/**
 * @brief Safe wrapper for checking if path is within root before file operations
 *
 * @param path The file path to check
 * @param root The root directory
 * @return true if safe, false if potentially malicious
 */
bool validateFilePath(const std::string& path, const std::string& root)
{
	// Additional checks before realpath resolution

	// Reject absolute paths that don't start with root
	if (path[0] == '/' && path.find(root) != 0)
	{
		std::cerr << "Security: Rejected absolute path outside root: " << path << std::endl;
		return false;
	}

	// Perform the full security check
	if (!isPathSafe(path, root))
	{
		std::cerr << "Security: Path traversal attempt detected!" << std::endl;
		std::cerr << "  Attempted path: " << path << std::endl;
		std::cerr << "  Allowed root: " << root << std::endl;
		return false;
	}

	return true;
}
