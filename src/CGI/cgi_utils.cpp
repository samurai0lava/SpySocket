#include "../../inc/webserv.hpp"


bool CGI::check_is_cgi(const ParsingRequest& request)
{
    std::string uri = request.getStartLine().at("uri");

    if (uri.find("/cgi-bin/") != 0)
    {
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

    std::string script_part = uri_without_query.substr("/cgi-bin/".length());

    size_t script_end = std::string::npos;
    std::vector<std::string> cgi_extensions;
    cgi_extensions.push_back(".cgi");
    cgi_extensions.push_back(".py");
    cgi_extensions.push_back(".pl");
    cgi_extensions.push_back(".sh");
    cgi_extensions.push_back(".php");

    // Find the end of the script name
    for (size_t i = 0; i < cgi_extensions.size(); ++i)
    {
        size_t ext_pos = script_part.find(cgi_extensions[i]);
        if (ext_pos != std::string::npos)
        {
            script_end = ext_pos + cgi_extensions[i].length();
            break;
        }
    }

    if (script_end != std::string::npos)
    {
        script_path = "/cgi-bin/" + script_part.substr(0, script_end);
        if (script_end < script_part.length())
            path_info = script_part.substr(script_end);
        else
            path_info = "";
    }
    else
    {
        // For scripts without recognized extensions (py, cgi, etc.)
        size_t slash_pos = script_part.find('/');
        if (slash_pos != std::string::npos)
        {
            script_path = "/cgi-bin/" + script_part.substr(0, slash_pos);
            path_info = script_part.substr(slash_pos);
        }
        else
        {
            script_path = "/cgi-bin/" + script_part;
            path_info = "";
        }
    }
    std::cout << "Script Path: " << script_path << std::endl;
    std::cout << "Path Info: " << path_info << std::endl;
    std::cout << "Query String: " << query_string << std::endl;
    return true;
}



