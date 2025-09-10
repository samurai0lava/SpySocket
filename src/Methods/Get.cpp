#include "../../inc/Get.hpp"
// #include "Get.hpp"


Get::Get(int client_fd,ParsingRequest* parser, ConfigStruct& config, Servers& serv, std::string uri):client_fd(client_fd), parser(parser), config(config), serv(serv), uri(uri)
{
 
}

Get::~Get()
{
}
void printLocationStruct(const LocationStruct& loc) {
    std::cout << "LocationStruct {" << std::endl;

    if(loc.autoIndex == true)
        std::cout<<"  autoIndex: true"<<std::endl;
    else 
        std::cout<<"  autoIndex: false"<<std::endl;

    std::cout << "  allowedMethods: ";
    for (std::set<std::string>::const_iterator it = loc.allowedMethods.begin();
         it != loc.allowedMethods.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    std::cout << "  root: " << loc.root << std::endl;
    std::cout << "  indexPage: " << loc.indexPage << std::endl;

    std::cout << "  _return: ";
    for (size_t i = 0; i < loc._return.size(); ++i) {
        std::cout << "[" << loc._return[i].first << " => " << loc._return[i].second << "] ";
    }
    std::cout << std::endl;

    std::cout << "  cgi_path: ";
    for (size_t i = 0; i < loc.cgi_path.size(); ++i) {
        std::cout << loc.cgi_path[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "  cgi_ext: ";
    for (size_t i = 0; i < loc.cgi_ext.size(); ++i) {
        std::cout << loc.cgi_ext[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "  upload_enabled: " << (loc.upload_enabled ? "true" : "false") << std::endl;
    std::cout << "  upload_path: " << loc.upload_path << std::endl;

    std::cout << "}" << std::endl;
}

string Get::MethodGet()
{
    std::string matchedLocation = matchLocation(this->uri , this->config);
    cout << "MATCHED LOC : " << matchedLocation << endl;
    if (!this->pathExists(matchedLocation)) 
    {
        cout << "111111111111\n"; 
        std::string finalResponse = GenerateResErr(404);
        return finalResponse;
        // return ;
    }
    // if (matchedLocation.empty()) 
    // {
    //     std::cout << "No matching location found.";
    //     throw std::runtime_error("");
    // }
    bool found = false;
    LocationStruct locationMatched;
    for (size_t i = 0; i < this->config.location.size(); ++i) {
        if (this->_name_location == this->config.location[i].first) {
            locationMatched = this->config.location[i].second;
            found = true;
            break;
        }
    }
    if (!found){ 
        std::cerr << "No exact match for location: " << matchedLocation << std::endl;
        throw std::runtime_error("");
    };
    if (!this->pathExists(matchedLocation)) 
    {
        cout << "2222222222\n";
        std::string finalResponse = GenerateResErr(404);
        // send(this->client_fd, finalResponse.c_str(), finalResponse.length(), 0);
        return finalResponse;
    }
    if (this->isFile(matchedLocation)) 
    {
        cout << "FIIIIIIIIIIILE\n";
        std::ifstream file(matchedLocation.c_str(), std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            std::string finalResponse = GenerateResErr(500);
            // send(this->client_fd, finalResponse.c_str(), finalResponse.length(), 0);
            // return ;
        return finalResponse;

        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string fileContent = buffer.str();
        file.close();
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: " << this->getMimeType(matchedLocation) << "\r\n";
        // response << "Connection: close" << "\r\n";
        response << "Content-Length: " << fileContent.size() << "\r\n\r\n";
        response << fileContent;
        std::string finalResponse = response.str();
        // send(this->client_fd, finalResponse.c_str(), finalResponse.length(), 0);
        return finalResponse;
        // return;
    }
    else if (this->isDirectory(matchedLocation)) 
    {
        std::string indexPath = matchedLocation + "/" + locationMatched.indexPage;
        if (this->pathExists(indexPath) && this->isFile(indexPath)) 
        {
            std::ifstream file(indexPath.c_str(), std::ios::in | std::ios::binary);
            if (!file.is_open())
            {
                std::string finalResponse = GenerateResErr(500);
                // send(this->client_fd, finalResponse.c_str(), finalResponse.length(), 0);
                return finalResponse;
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string fileContent = buffer.str();
            file.close();

            std::ostringstream response;
            response << "HTTP/1.1 200 OK\r\n";
            // response << "Content-Type: "<< this->getMimeType(matchedLocation)<<"\r\n";
            response << "Content-Type: "<< "text/html"<<"\r\n";
            // std::cout << "Serving file: " << matchedLocation << std::endl;
            // std::cout << this->getMimeType(matchedLocation) << std::endl;
            response << "Content-Length: " << fileContent.size() << "\r\n\r\n";
            response << fileContent;

            std::string finalResponse = response.str();
            // send(this->client_fd, finalResponse.c_str(), finalResponse.length(), 0);
            // return;
        return finalResponse;

        }
        else if (locationMatched.autoIndex == true ) 
        {
            std::string listing = this->generateAutoIndex(matchedLocation);
            std::ostringstream response;
            response << "HTTP/1.1 200 OK\r\n";
            response << "Content-Type: " <<"text/html"<<"\r\n";
            response << "Content-Length: " << listing.size() << "\r\n\r\n";
            response << listing;

            std::string finalResponse = response.str();
            // send(this->client_fd, finalResponse.c_str(), finalResponse.length(), 0);
            // return;
        return finalResponse;

        }
        else 
        {
            std::string finalResponse = GenerateResErr(403);
            // send(this->client_fd, finalResponse.c_str(), finalResponse.length(), 0);
            // return ;
            return finalResponse;

        }
    }
    return "";
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
        for (size_t i = 0; i < server.location.size(); ++i) {
            if (path == server.location[i].first) {
                
                if(server.location[i].second.allowedMethods.find("GET") == server.location[i].second.allowedMethods.end())
                {
                    //should send a 405 Method Not Allowed response
                    throw runtime_error("Error 405 Method Not Allowed");
                }
                this->_name_location = server.location[i].first;
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
    html << "<!DOCTYPE html>\n";
    html << "<html>\n<head>\n";
    html << "<meta charset=\"UTF-8\">\n";
    html << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    html << "<title>Index of " << directoryPath << "</title>\n";
    html << "<style>\n";
    html << "body { font-family: Arial, sans-serif; margin: 40px; }\n";
    html << "h1 { color: #333; border-bottom: 1px solid #ccc; padding-bottom: 10px; }\n";
    html << "ul { list-style: none; padding: 0; }\n";
    html << "li { margin: 5px 0; }\n";
    html << "a { text-decoration: none; color: #0066cc; padding: 5px; display: block; }\n";
    html << "a:hover { background-color: #f0f0f0; }\n";
    html << ".directory { font-weight: bold; }\n";
    html << "</style>\n";
    html << "</head>\n<body>\n";
    html << "<h1>Index of " << directoryPath << "</h1>\n";
    html << "<ul>\n";

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
