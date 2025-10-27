#ifndef WEBSERV_HPP
# define WEBSERV_HPP


# define DEBUG_MODE 1

#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm>
#include <cctype>
#include <sys/wait.h>
#include <signal.h>
#include <ctime>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include "Methods.hpp"
#include "parsing_request.hpp"
#include "../inc/Config.hpp"
#include "../inc/server.hpp"
#include "../inc/singleserver.hpp"
#include "../inc/Struct.hpp"
#include "Get.hpp"
#include "RespondError.hpp"
#include "MethodHandler.hpp"
#include "CGI.hpp"
#include "Delete.hpp"
#include "ft_time.hpp"
#include "logs.hpp"
#include "POST.hpp"
#include "CClient.hpp"
#include "CookieManager.hpp"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

// Security functions
bool isPathSafe(const std::string& path, const std::string& root);
bool validateFilePath(const std::string& path, const std::string& root);
char* ft_realpath(const char* path, char* resolved_path);


#endif

