#include "../../inc/webserv.hpp"


bool CGI::check_is_cgi(const ParsingRequest& request)
{
    std::string uri = request.getStartLine().at("uri"); 
    size_t ext_pos = uri.find(".cgi");
    if (ext_pos != std::string::npos)
    {
        return true;
    }
    ext_pos = uri.find(".py");
    if (ext_pos != std::string::npos)
    {
        return true;
    }
    ext_pos = uri.find(".php");
    if (ext_pos != std::string::npos)
    {
        return true;
    }
    return false;
}