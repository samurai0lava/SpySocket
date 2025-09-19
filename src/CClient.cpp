#include "../inc/CClient.hpp"
#include "../inc/Get.hpp"
#include "../inc/webserv.hpp"

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
    // cout << "PARSER CCLIENT : " << parser->getHeaders()["connection"] << endl;
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
    // cout << "PARSER METHOD : " << parser->getHeaders()["connection"] << endl;
    if (this->NameMethod == "GET")
    {

        Get _MGet(*this);
        try {
            return (_MGet.MethodGet(*this->parser));
        }
        catch (const std::runtime_error& e) {
            std::string errMsg = e.what();
            if (errMsg.find("405") != std::string::npos) {
                return GenerateResErr(405);
            }
            else if (errMsg.find("403") != std::string::npos) {
                return GenerateResErr(403);
            }
            else {
                return GenerateResErr(500); // Generic server error for other exceptions
            }
        }
    }
    else if (this->NameMethod == "DELETE")
    {
        DeleteMethode MDelete;
        return (MDelete.PerformDelete(this->uri, this->mutableConfig));
    }
    else if(this->NameMethod == "POST")
    {
        return postMethod(this->uri, this->mutableConfig, *this->parser);
    }
    else
        return GenerateResErr(405); // Method Not Allowed
    return string();
}



