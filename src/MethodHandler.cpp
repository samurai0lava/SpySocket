#include "../inc/MethodHandler.hpp"
bool pathExists(const std::string& path) {
    struct stat s;
    return stat(path.c_str(), &s) == 0;
}
bool isFile(const std::string& path) {
    struct stat s;
    return stat(path.c_str(), &s) == 0 && S_ISREG(s.st_mode);
}

bool isDirectory(const std::string& path) {
    struct stat s;
    return stat(path.c_str(), &s) == 0 && S_ISDIR(s.st_mode);
}
std::string generateAutoIndex(const std::string &directoryPath) 
{
    DIR *dir = opendir(directoryPath.c_str());
    if (!dir)
        return "<html><body><h1>403 Forbidden</h1></body></html>";

    std::ostringstream html;
    html << "<html><head><title>Index of " << directoryPath << "</title></head><body>\n";
    html << "<h1>Index of " << directoryPath << "</h1><hr><ul>\n";

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;

        if (name == ".") continue; // skip current dir

        std::string fullPath = directoryPath + "/" + name;
        struct stat s;
        stat(fullPath.c_str(), &s);

        if (S_ISDIR(s.st_mode)) {
            html << "<li><a href=\"" << name << "/\">" << name << "/</a></li>\n";
        } else {
            html << "<li><a href=\"" << name << "\">" << name << "</a></li>\n";
        }
    }

    closedir(dir);

    html << "</ul><hr></body></html>\n";
    return html.str();
}
//-------------------------------------------------------------------------------------------
std::string matchLocation(const std::string &requestPath, const ConfigStruct &server) {
    std::string path = requestPath;
    std::string removedSegment;
    std::string removedPath; 


    while (true) {
        std::cout << "Path : " << path << " removed : " << removedPath << std::endl;

        for (size_t i = 0; i < server.location.size(); ++i) {
            if (path == server.location[i].first) {
                
                if(server.location[i].second.allowedMethods.find("GET") == server.location[i].second.allowedMethods.end())
                {
                    throw runtime_error("Error 405 Method Not Allowed");
                }
                else 
                    cout<<"found "<<endl;
                return server.location[i].second.root +removedPath;
            }
        }
        if (path == "/")
            break;
        size_t lastSlash = path.find_last_of('/');
        if (lastSlash == std::string::npos || lastSlash == 0) 
        {
            removedSegment = path.substr(lastSlash);
            removedPath = removedSegment + removedPath;
            path = "/";
        } else 
        {
            removedSegment = path.substr(lastSlash); 
            removedPath = removedSegment + removedPath; 
            path = path.substr(0, lastSlash);
        }
    }

    return ""; 
}
void MethodGet(std::string Path,const ConfigStruct& config,Servers &serv)
{
    std::string matchedLocation = matchLocation(Path, config);
    cout<<" : matching : "<<matchedLocation<<endl;
    if (matchedLocation.empty()) 
    {
       std::cout << "No matching location found." << std::endl;
       return;
    }
    if (!pathExists(matchedLocation)) 
    {
        // cout<<"path exists : "<<matchedLocation<<endl;
        throw std::runtime_error("404 Not Found: Path does not exist");
    }
    if (isFile(matchedLocation)) 
    {
        std::ifstream file(matchedLocation.c_str(), std::ios::in | std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("500 Internal Server Error: Cannot open file");
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string fileContent = buffer.str();
        file.close();
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: text/html\r\n";
        response << "Content-Length: " << fileContent.size() << "\r\n";
        response << "\r\n";
        response << fileContent;

        std::string finalResponse = response.str();
        // std::cout<<"final Response: \n"<<finalResponse<<endl;
        send(serv.getServersFds()[0], finalResponse.c_str(), finalResponse.length(), 0);
        return;
    }
    if (isDirectory(matchedLocation)) 
    {
        // std::cout << "matchedLocation: " << matchedLocation << std::endl;
        // std::cout << "indexPage: " << config.location[0].second.indexPage << std::endl;
        // std::cout<<"autoindex: "<<config.location[0].second.autoIndex <<endl;
        std::string indexPath = matchedLocation + "/" + config.location[0].second.indexPage;
        if (!pathExists(indexPath) ) 
        {
            std::ifstream file(indexPath.c_str(), std::ios::in | std::ios::binary);
            if (!file.is_open())
                throw std::runtime_error("500 Internal Server Error: Cannot open index file");
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string fileContent = buffer.str();
            file.close();
            std::ostringstream response;
            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: text/html\r\n";
            response << "Content-Length: " << fileContent.size() << "\r\n";
            response << "\r\n";
            response << fileContent;
            std::string finalResponse = response.str();
            // std::cout << "Final Response (index file): \n" << finalResponse << std::endl;
            send(serv.getServersFds()[0], finalResponse.c_str(), finalResponse.length(), 0);
            return;
        }
        else if (config.location[0].second.autoIndex == true )
        {
            // cout<<"-----------autoIndex-----------"<<endl;
            // std::cout<<"print auto index : "<< config.location[0].second.autoIndex <<endl;
            // // Generate HTML autoindex
            // cout<<"lol123"<<endl;
            std::string listing = generateAutoIndex(matchedLocation);
            std::ostringstream response;
            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: text/html\r\n";
            response << "Content-Length: " << listing.size() << "\r\n\r\n";
            response << listing;
            std::string finalResponse = response.str();
            // cout<<"--------------response--------"<<endl;
            // std::cout << "Autoindex Response:\n" << finalResponse<< std::endl;
            send(serv.getServersFds()[0], finalResponse.c_str(), finalResponse.length(), 0);
            return;
        } 
        else 
            throw std::runtime_error("403 Forbidden: Index not found and autoindex is off");
    }
}

void handleMethod(int fd, ParsingRequest* parser, const ConfigStruct& config,Servers &serv)
{
    (void)fd;
    (void)config;
    std::string method = parser->getStartLine()["method"];
    std::string uri = parser->getStartLine()["uri"];
    if (method == "GET") 
    {
        cout<<"uri : "<<uri<<endl;
        MethodGet(uri,config,serv);
    }
}