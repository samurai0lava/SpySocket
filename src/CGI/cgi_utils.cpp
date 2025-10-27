#include "../../inc/webserv.hpp"

bool CGI::isExtensionAllowed(const std::string& scriptPath, const LocationStruct& location) const {
    if (location.cgi_ext.empty()) {
        return true;
    }
    size_t dotPos = scriptPath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return false;
    }

    std::string extension = scriptPath.substr(dotPos);
    for (size_t i = 0; i < location.cgi_ext.size(); ++i) {
        if (location.cgi_ext[i] == extension) {
            return true;
        }
    }

    return false;
}

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

    if (!isExtensionAllowed(script_path, current_location))
    {
        is_cgi = 0;
        error_code = 403;
        error_message = "CGI script extension not allowed";
        return false;
    }

    return true;
}
