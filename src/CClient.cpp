#include "../inc/CClient.hpp"
#include "../inc/Get.hpp"

CClient::CClient() :
    _name_location(""), NameMethod(""), uri(""), FdClient(-1), mutableConfig(), serv(), parser(NULL),
    SendHeader(false), readyToSendAllResponse(false), chunkedSending(false), 
    chunkSize(0), bytesSent(0), response(""), filePath(""), fileSize(0), 
    offset(0), fileFd(-1), intialized(false), Chunked(false)
{

}

CClient::CClient(string NameMethod, string uri, int FdClient, ConfigStruct MConfig, Servers serv, ParsingRequest* parser) :
    _name_location(""), NameMethod(NameMethod), uri(uri), FdClient(FdClient), mutableConfig(MConfig), serv(serv), parser(parser),
    SendHeader(false), readyToSendAllResponse(false), chunkedSending(false), chunkSize(0), bytesSent(0),
    response(""), filePath(""), fileSize(0), offset(0), fileFd(-1), intialized(false), Chunked(false)

{
}

CClient::~CClient()
{
}
void CClient::printInfo() const {
    std::cout << "===== Client Info =====" << std::endl;
    std::cout << "Method: " << NameMethod << std::endl;
    std::cout << "URI: " << uri << std::endl;
    std::cout << "FdClient: " << FdClient << std::endl;
    // std::cout << "Read Buffer: " << readbuf << std::endl;
    // std::cout << "Offset: " << offset << std::endl;
    // std::cout << "Size File: " << sizeFile << std::endl;

    // Optional: print parser address
    std::cout << "Parser ptr: " << parser << std::endl;

    // Config info (if you want details from mutableConfig)
    std::cout << "Config root: " << mutableConfig.root << std::endl;
    std::cout << "Config indexPage: " << mutableConfig.indexPage << std::endl;

    std::cout << "=======================" << std::endl;
}


string CClient::HandleAllMethod()
{
    if (this->NameMethod == "GET")
    {

        Get _MGet(*this);
        try {
            return (_MGet.MethodGet());
        }
        catch (const std::runtime_error& e) {
            std::string errMsg = e.what();
            if (errMsg.find("405") != std::string::npos) {
                return getErrorPageFromConfig(405);
            }
            else if (errMsg.find("403") != std::string::npos) {
                return getErrorPageFromConfig(403);
            }
            else {
                return getErrorPageFromConfig(500); // Generic server error for other exceptions
            }
        }
    }
    else if (this->NameMethod == "DELETE")
    {
        DeleteMethode MDelete;
        return (MDelete.PerformDelete(this->uri, this->mutableConfig));
    }
    //  else if(this->NameMethod == "POST")
    //  {}
    else
        return getErrorPageFromConfig(405); // Method Not Allowed
    return string();
}

std::string CClient::getErrorPageFromConfig(int statusCode)
{
    for (size_t i = 0; i < this->mutableConfig.errorPage.size(); ++i)
    {
        if (std::atoi(this->mutableConfig.errorPage[i].first.c_str()) == statusCode)
        {
            std::string errorPagePath = this->mutableConfig.root + this->mutableConfig.errorPage[i].second;
            
            std::ifstream file(errorPagePath.c_str(), std::ios::in | std::ios::binary);
            if (file.is_open())
            {
                std::stringstream buffer;
                buffer << file.rdbuf();
                file.close();
                
                std::ostringstream response;
                std::string statusMessage = getStatusPhrase(statusCode);
                response << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
                response << "Content-Type: text/html\r\n";
                response << "Content-Length: " << buffer.str().size() << "\r\n\r\n";
                response << buffer.str();
                return response.str();
            }
        }
    }
    return GenerateResErr(statusCode);
}



