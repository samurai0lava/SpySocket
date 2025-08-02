#include "../../inc/Get.hpp"

Get::Get(int client_fd,ParsingRequest* parser, ConfigStruct& config, Servers& serv, std::string uri):client_fd(client_fd), parser(parser), config(config), serv(serv), uri(uri)
{
 
}

Get::~Get()
{
}

void Get::MethodGet()
{
    std::string matchedLocation = matchLocation(this->uri , this->config);
    if (matchedLocation.empty()) 
    {
        std::cout << "No matching location found." << std::endl;
        return;
    }
    LocationStruct locationMatched;
    for (size_t i = 0; i < this->config.location.size(); ++i) {
        if (matchedLocation == this->config.location[i].first) {
            locationMatched = this->config.location[i].second;
            break;
        }
    }

    std::cout<<" fd : "<<this->serv.serversFd[0]<< endl;
    cout<<"path = " <<matchedLocation<<endl;
    if (!this->pathExists(matchedLocation)) 
        throw std::runtime_error("404 Not Found: Path does not exist");
    if (this->isFile(matchedLocation)) 
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
        response << "Content-Type: " << this->getMimeType(matchedLocation) << "\r\n";
        response << "Content-Length: " << fileContent.size() << "\r\n";
        // response << "Content-Length: " << b.size() << "\r\n";
        response << "Connection: keep-alive\r\n"; 
        response << "\r\n"; 
        response << fileContent;
        std::string finalResponse = response.str();
        std::cout<<"is file : "<<response.str()<<endl;
        // std::cout<<"server fd : "<<serv.getServersFds()[0]<<std::endl;
        send(this->client_fd, finalResponse.c_str(), finalResponse.length(), 0);
        return;
    }
    if (this->isDirectory(matchedLocation)) 
    {
        std::string indexPath = matchedLocation + "/" + locationMatched.indexPage;

        if (this->pathExists(indexPath) && this->isFile(indexPath)) 
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
            response << "Content-Type: "<< this->getMimeType(matchedLocation)<<"\r\n";
            response << "Content-Length: " << fileContent.size() << "\r\n\r\n";
            response << fileContent;

            std::string finalResponse = response.str();
            send(this->client_fd, finalResponse.c_str(), finalResponse.length(), 0);
            return;
        }
        else if (locationMatched.autoIndex) 
        {
            std::string listing = this->generateAutoIndex(matchedLocation);
            std::ostringstream response;
            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: "<<this->getMimeType(matchedLocation)<<"\r\n";
            response << "Content-Length: " << listing.size() << "\r\n\r\n";
            response << listing;

            std::string finalResponse = response.str();
            send(this->client_fd, finalResponse.c_str(), finalResponse.length(), 0);
            return;
        }
        else 
            throw std::runtime_error("403 Forbidden: Index not found and autoindex is off");
    }
}

std::string Get::getMimeType(const std::string& path)
{
    size_t dot = path.find_last_of(".");
    if (dot == std::string::npos) return "text/plain";

    std::string ext = path.substr(dot);
    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".css") return "text/css";
    if (ext == ".js") return "application/javascript";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    return "application/octet-stream";
}

std::string Get::matchLocation(const std::string& requestPath, const ConfigStruct& server)
{
     std::string path = requestPath;
    std::string removedSegment;
    std::string removedPath; 
    while (true) {
        // std::cout << "Path : " << path << " removed : " << removedPath << std::endl;

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

std::string Get::generateAutoIndex(const std::string& directoryPath)
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

bool Get::isDirectory(const std::string& path)
{
    struct stat s;
    return stat(path.c_str(), &s) == 0 && S_ISDIR(s.st_mode);
}

bool Get::isFile(const std::string& path)
{
    struct stat s;
    return stat(path.c_str(), &s) == 0 && S_ISREG(s.st_mode);
}

bool Get::pathExists(const std::string& path)
{
   struct stat s;
   return stat(path.c_str(), &s) == 0;
}
