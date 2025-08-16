#include <iostream>
#include <vector>

std::vector<std::string> split(std::string s, std::string delimiters)
{
	std::vector<std::string> tokens;
	std::string token;
	for (std::string::size_type i = 0; i < s.size(); ++i)
	{
		if (delimiters.find(s[i]) != std::string::npos)
		{
			if (!token.empty())
			{
				tokens.push_back(token);
				token.clear();
			}
		}
		else
		{
			token += s[i];
		}
	}
	if (!token.empty())
	{
		tokens.push_back(token);
	}
	return (tokens);
}


std::string boundary = "------WebKitFormBoundaryX";
std::string body = requestBody;

size_t pos = 0;
while ((pos = body.find(boundary, pos)) != std::string::npos) {
    pos += boundary.size();

    // Skip leading CRLF
    if (body.compare(pos, 2, "\r\n") == 0)
        pos += 2;

    // Find end of headers
    size_t headerEnd = body.find("\r\n\r\n", pos);
    if (headerEnd == std::string::npos)
        break;

    std::string headers = body.substr(pos, headerEnd - pos);

    // Content starts after headerEnd+4
    size_t contentStart = headerEnd + 4;

    // Find next boundary
    size_t nextBoundary = body.find(boundary, contentStart);
    std::string content = body.substr(contentStart, nextBoundary - contentStart);

    std::cout << "Headers:\n" << headers << "\n";
    std::cout << "Content:\n" << content << "\n";

    pos = nextBoundary;
}



int main()
{
    std::vector<std::string> splitted = split("hello world", " ");
    
    for(std::vector<std::string>::iterator it = splitted.begin(); it != splitted.end(); it++)
        std::cout << *it << std::endl;
}