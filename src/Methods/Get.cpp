#include "../../inc/Get.hpp"


Get::Get(CClient& c) : client(c) {}


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
std::string Get::getMimeType(const std::string& path)
{
    size_t dot = path.find_last_of(".");
    if (dot == std::string::npos) return "text/html";

    std::string ext = path.substr(dot);
    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".css") return "text/css";
    if (ext == ".mp4") return "video/mp4";
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
                    throw runtime_error("Error 405 Method Not Allowed");
                }
                this->client._name_location = server.location[i].first;
                // printLocationStruct(server.location[i].second);
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

std::string Get::buildHttpHeaders(const std::string& path, size_t fileSize)
{
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n";
    oss << "Content-Type: " << this->getMimeType(path) << "\r\n";
    oss << "Content-Length: " << fileSize << "\r\n\r\n";
    return oss.str();
}
string Get::pathIsFile(string matchLocation)
{
    this->client.filePath = matchLocation;
    
    // Get file size using stat
    struct stat fileStat;
    if (stat(matchLocation.c_str(), &fileStat) == -1) {
        cerr << "Error getting file stats!" << endl;
        return GenerateResErr(500);
    }
    
    // Check if file is larger than 1MB, use chunked sending
    if(fileStat.st_size > 1024*1024)
    {
       
        if (client.intialized == false)
        {
            client.intialized = true;
            client.chunkSize = 1024 ;
            client.bytesSent = 0;
            client.fileSize = fileStat.st_size;
            client.chunkedSending = false;
            client.SendHeader = false;
            client.fileFd = open(client.filePath.c_str(), O_RDONLY);
            if (client.fileFd == -1) {
                cerr << "Error opening file for chunked sending!" << endl;
                return GenerateResErr(500) ;
            }
            else
                return (setupChunkedSending(client.filePath));
            
        }
        return (setupChunkedSending(client.filePath));
    }
    
    // For small files, read normally
    ifstream file(matchLocation.c_str(),std::ios::in | std::ios::binary);
    if(!file.is_open())
    {
        string finalResponse = GenerateResErr(500);
        return (finalResponse);
    }
    stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    ostringstream response;
    response<<"HTTP/1.1 200 OK \r\n";
    response<<"Content-type: "<<this->getMimeType(matchLocation)<<"\r\n";
    response<<"Content-length: "<<buffer.str().size()<<"\r\n\r\n";
    response<<buffer.str();
    return (response.str());
}

string Get::handleDirectoryWithIndex(string indexPath)
{
    ifstream file(indexPath.c_str(), std::ios::in | std::ios::binary);
    if(!file.is_open())
        return (GenerateResErr(500));
    stringstream buffer;
    buffer<<file.rdbuf();
    file.close();

    ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: " << this->getMimeType(indexPath) << "\r\n";
    response << "Content-Length: " << buffer.str().size() << "\r\n\r\n";
    response << buffer.str();
    return (response.str());
}
string Get::handleDirectoryWithAutoIndex(string matchLocation)
{
    string listing = this->generateAutoIndex(matchLocation);
    ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: " <<"text/html"<<"\r\n";
    response << "Content-Length: " << listing.size() << "\r\n\r\n";
    response << listing; 
    return (response.str());
}
string Get::MethodGet()
{
    if(this->client.uri.empty() ){
        std::cerr << "Empty URI in GET method" << std::endl;
        return (GenerateResErr(400));}
    string matchedLocation = matchLocation(this->client.uri , this->client.mutableConfig);
    if(!this->pathExists(matchedLocation))
    {
        string finalResponce = GenerateResErr(404);
        return (finalResponce);
    }
    bool found = false ;
    LocationStruct locationMatched;
    for(size_t i = 0; i < this->client.mutableConfig.location.size(); i++)
    {
        locationMatched = this->client.mutableConfig.location[i].second;
        found = true ;
        break ;
    }
    if(!found)
    {
        std::cerr << "No exact match for location : "<< matchedLocation << std::endl;
        throw std::runtime_error("");
    }
    if (!locationMatched._return.empty()) {
        int statusCode = atoi(locationMatched._return[0].first.c_str());
        std::string target = locationMatched._return[0].second;
        return buildRedirectResponse(statusCode, target);
    }
    if(this->isFile(matchedLocation)){
        return(pathIsFile(matchedLocation));}
    else if(this->isDirectory(matchedLocation))
    {
        string indexPath = matchedLocation + "/" + locationMatched.indexPage;
        if(this->pathExists(indexPath) && this->isFile(indexPath))
            return (this->handleDirectoryWithIndex(indexPath));
        else if(locationMatched.autoIndex == true )
            return (this->handleDirectoryWithAutoIndex(matchedLocation));
    }
    return (GenerateResErr(403));
}


string Get::setupChunkedSending(const std::string& filePath)
{
    
    if( this->client.SendHeader == false)
    {
        
        struct stat s;
        if (stat(filePath.c_str(), &s) == -1) {
            return GenerateResErr(500);
        }
        this->client.fileSize = s.st_size;
        std::ostringstream oss;
        oss << "HTTP/1.1 200 OK\r\n";
        oss << "Content-Type: " << getMimeType(filePath) << "\r\n";
        oss << "Transfer-Encoding: chunked\r\n";
        oss << "\r\n";
        this->client.response = oss.str();
        this->client.SendHeader = true; // Ensure headers are sent only once
    }
    else
    {
        // std::cout<<"setupChunkedSending called again\n";
        char buffer[this->client.chunkSize + 1];
        ssize_t bytesRead = read(this->client.fileFd, buffer, this->client.chunkSize);
        if (bytesRead == -1) {
            close(this->client.fileFd);
            return GenerateResErr(500);
        } else if (bytesRead == 0) {
            this->client.response = "0\r\n\r\n";
            close(this->client.fileFd);
            this->client.chunkedSending = true; 
        } else {
            std::ostringstream oss;
            oss << std::hex << bytesRead << "\r\n"; 
            oss.write(buffer, bytesRead);
            oss << "\r\n";
            this->client.response = oss.str();
            // this->client.bytesSent += bytesRead;
        }
    }
    return this->client.response;
}

void Get::printLocationStruct(const LocationStruct& loc)
{
    std::cout << "  LocationStruct {" << std::endl;
    std::cout << "    autoIndex: " << (loc.autoIndex ? "true" : "false") << std::endl;

    std::cout << "    allowedMethods: ";
    for (std::set<std::string>::const_iterator it = loc.allowedMethods.begin();
         it != loc.allowedMethods.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    std::cout << "    root: " << loc.root << std::endl;
    std::cout << "    indexPage: " << loc.indexPage << std::endl;
    if (!loc.cgi_ext.empty())
        std::cout << "    cgi_ext: " << loc.cgi_ext[0] << std::endl;
    else
        std::cout << "    cgi_ext: (empty)" << std::endl;
    std::cout << "    upload_enabled: " << (loc.upload_enabled ? "true" : "false") << std::endl;
    std::cout << "    upload_path: " << loc.upload_path << std::endl;
    std::cout << "  }" << std::endl;
}

string Get::buildRedirectResponse(int statusCode, const std::string& target)
{
     std::ostringstream oss;

    // Choose correct status message
    std::string statusMessage = (statusCode == 301) ? "Moved Permanently" :
                                (statusCode == 302) ? "Found" :
                                (statusCode == 307) ? "Temporary Redirect" :
                                "Redirect";

    oss << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    oss << "Location: " << target << "\r\n";
    oss << "Content-Length: 0\r\n";
    oss << "Connection: close\r\n\r\n";

    return oss.str();
}
