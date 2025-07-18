#include "inc/webserv.hpp"

int main()
{

	std::string request_dummy = "GET /index.html HTTP/1.1\r\n"
								"Host: localhost:8080\r\n"
								"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3\r\n"
								"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
								"Accept-Language: en-US,en;q=0.5\r\n"
								"Accept-Encoding: gzip, deflate\r\n"
								"Connection: keep-alive\r\n\r\n";

	ParsingRequest request;

	std::string start_line_str = request.get_start_line(request_dummy);
	std::map<std::string, std::string> start_line = request.split_start_line(start_line_str);

	if (!request.checkMethod(start_line) || !request.checkURI(start_line) || !request.checkVersion(start_line))
	{
		std::cerr << "Invalid request format." << std::endl;
		return 1;
	}

	std::cout << "Request Method: " << start_line["method"] << std::endl;
	std::cout << "Request URI: " << start_line["uri"] << std::endl;
	std::cout << "Request Version: " << start_line["version"] << std::endl;

	return 0;
}
