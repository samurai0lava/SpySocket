#include "singleserver.hpp"

void printConfigStruct(const ConfigStruct &conf)
{
    std::cout << "\n=== SERVER CONFIG ===\n";

    std::cout << "Server Name: " << conf.serverName << std::endl;

    std::cout << "Root: " << conf.root << std::endl;
    std::cout << "Host: " <<conf.host << std::endl;

    std::cout << "Index Page: " << conf.indexPage << std::endl;
    std::cout << "client_max_body_size : "<<conf.clientMaxBodySize<<std::endl;

    std::cout << "Auto Index: " << (conf.autoIndex ? "on" : "off") << std::endl;

    std::cout << "\n--- Listen ---\n";
    for (std::vector<unsigned short>::const_iterator it = conf.listen.begin();
    it != conf.listen.end(); ++it)
{
   std::cout << "Port: " << *it << std::endl;
}

    std::cout << "\n--- Error Pages ---\n";
    for (size_t i = 0; i < conf.errorPage.size(); ++i)
    {
        std::cout << "Error " << conf.errorPage[i].first
                  << " => " << conf.errorPage[i].second << std::endl;
    }

    std::cout << "\n--- Locations ---\n";
    for (std::vector<std::pair<std::string, LocationStruct> >::const_iterator it = conf.location.begin();
        it != conf.location.end(); ++it)
    {
        std::cout << " _____Location Path____: " << it->first << std::endl;
        const LocationStruct &loc = it->second;

        std::cout << "\t  Root: " << loc.root << std::endl;
        if(!loc.indexPage.empty())
            std::cout << "\t  Index Page: " << loc.indexPage << std::endl;
        std::cout << "\t  Auto Index: " << (loc.autoIndex ? "on" : "off") << std::endl;
        if(!loc._return.empty())
         std::cout << "\t  _return : " << loc._return << std::endl;
        if(!loc.allowedMethods.empty()){
            std::cout << "\t  Allowed Methods: ";
            for (std::set<std::string>::const_iterator mit = loc.allowedMethods.begin();
                mit != loc.allowedMethods.end(); ++mit)
            {
                std::cout << *mit << " ";
            }
            std::cout << std::endl;}
        
        if(loc.cgi_ext.size() != 0)
        {    
        std::cout << "\t  CGI Paths: ";
        for (size_t i = 0; i < loc.cgi_path.size(); ++i)
        {
            std::cout << loc.cgi_path[i] << " ";
        }
        std::cout << std::endl;
        }


        if(loc.cgi_ext.size() != 0){
        std::cout << "\t  CGI Extension: ";
        for (size_t i = 0; i < loc.cgi_ext.size(); ++i)
        {
            std::cout << loc.cgi_ext[i] << " ";
        }
        std::cout << std::endl;}
        

    }
}
