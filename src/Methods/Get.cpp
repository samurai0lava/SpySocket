#include "../../inc/Get.hpp"
// #include "Get.hpp"


Get::Get(CClient& c) : client(c) {}
Get::Get()
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
                this->_name_location = server.location[i].first;
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
    this->filePath = matchLocation;
    
    // Get file size using stat
    struct stat fileStat;
    if (stat(matchLocation.c_str(), &fileStat) == -1) {
        cerr << "Error getting file stats!" << endl;
        return GenerateResErr(500);
    }
    
    // Check if file is larger than 1MB, use chunked sending
    if(fileStat.st_size > 1024*1024)
    {
        if(this->SendHeader == true)
          std::cout <<"Headers already sent for chunked response : true"<<std::endl;
        else
          std::cout <<"Sending headers for chunked response : false "<<std::endl;
        std::cout<<"im here in PathIsFile for chunked sending"<<std::endl;
        cout << "File size: " << fileStat.st_size << " bytes, using chunked sending" << endl;
        if (chunkedSending == false)
        {
            this->chunkedSending = true;
            this->chunkSize = 1024;
            this->bytesSent = 0;
            this->fileSize = fileStat.st_size;
            this->fileFd = open(this->filePath.c_str(), O_RDONLY);
            if (this->fileFd == -1) {
                cerr << "Error opening file for chunked sending!" << endl;
                return GenerateResErr(500) ;
            }
            else 
                return (setupChunkedSending(this->filePath));
            
        }
        return (setupChunkedSending(this->filePath));
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
    if(this->uri.empty() ){
        std::cerr << "Empty URI in GET method" << std::endl;
        return (GenerateResErr(400));}
    string matchedLocation = matchLocation(this->uri , this->mutableConfig);
    if(!this->pathExists(matchedLocation))
    {
        string finalResponce = GenerateResErr(404);
        return (finalResponce);
    }
    bool found = false ;
    LocationStruct locationMatched;
    for(size_t i = 0; i < this->mutableConfig.location.size(); i++)
    {
        locationMatched = this->mutableConfig.location[i].second;
        found = true ;
        break ;
    }
    if(!found)
    {
        std::cerr << "No exact match for location : "<< matchedLocation << std::endl;
        throw std::runtime_error("");
    }
    if(this->isFile(matchedLocation)){
        std::cout<<"it's a file"<<std::endl;
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
    std::cout<<"Setting up chunked sending for file: " << filePath << std::endl;
    std::cout<<" this->SendHeader : "<< this->SendHeader << std::endl;
    if( this->SendHeader == false)
    {
        std::cout<<"Sending headers for chunked response"<<std::endl;
        struct stat s;
        if (stat(filePath.c_str(), &s) == -1) {
            return GenerateResErr(500);
        }
        this->fileSize = s.st_size;
        std::ostringstream oss;
        oss << "HTTP/1.1 200 OK\r\n";
        // oss << "Content-Type: " << getMimeType(filePath) << "\r\n";
        oss << "Transfer-Encoding: chunked\r\n";
        oss << "\r\n";
        this->response += oss.str();
        this->SendHeader = true; // Ensure headers are sent only once
        std::cout <<" 2222222222222heder sent  : "<< this->SendHeader << std::endl;
    }
    else
    {
        std::cout<<"Continuing chunked sending for file: " << filePath << std::endl;
        char buffer[this->chunkSize + 1];
        ssize_t bytesRead = read(this->fileFd, buffer, this->chunkSize);
        if (bytesRead == -1) {
            close(this->fileFd);
            return GenerateResErr(500);
        } else if (bytesRead == 0) {
            // End of file reached, send final chunk
            this->response += "0\r\n\r\n";
            close(this->fileFd);
            this->chunkedSending = false; // Finished sending
        } else {
            buffer[bytesRead] = '\0';
            std::ostringstream oss;
            oss << std::hex << bytesRead << "\r\n"; // Chunk size in hex
            oss << std::string(buffer, bytesRead) << "\r\n"; // Chunk data
            this->response += oss.str();
            this->bytesSent += bytesRead;
        }
    }
    return this->response;
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
