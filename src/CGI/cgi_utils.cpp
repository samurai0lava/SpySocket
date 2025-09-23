#include "../../inc/webserv.hpp"


bool CGI::check_is_cgi(const ParsingRequest& request)
{
    std::map<std::string, std::string> startLine = request.getStartLine();
    if (startLine.find("uri") == startLine.end())
    {
        is_cgi = 0;
        return false;
    }
    
    std::string uri = startLine.at("uri");

    if (uri.find("/cgi-bin/") != 0)
    {
        std::cout << RED "alklo" RESET << std::endl;
        is_cgi = 0;
        return false;
    }

    is_cgi = 1;
    size_t query_pos = uri.find('?');
    std::string uri_without_query;

    if (query_pos != std::string::npos)
    {
        query_string = uri.substr(query_pos + 1);
        uri_without_query = uri.substr(0, query_pos);
    }
    else
    {
        query_string = "";
        uri_without_query = uri;
    }

    std::string cgi_prefix = "/cgi-bin/";
    std::string script_part = uri_without_query.substr(cgi_prefix.length());

    // Treat any file in cgi-bin as executable, regardless of extension
    // Find the first slash to separate script name from path_info
    size_t slash_pos = script_part.find('/');
    if (slash_pos != std::string::npos)
    {
        script_path = cgi_prefix + script_part.substr(0, slash_pos);
        path_info = script_part.substr(slash_pos);
    }
    else
    {
        script_path = cgi_prefix + script_part;
        path_info = "";
    }
    return true;
}



