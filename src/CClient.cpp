#include "../inc/CClient.hpp"
#include "../inc/Get.hpp"
// #include "CClient.hpp"
// #include "../inc/RespondError.hpp"
CClient::CClient()
{
    
}

CClient::CClient(string NameMethod, string uri,int FdClient, ConfigStruct MConfig, Servers serv, ParsingRequest* parser):
    NameMethod(NameMethod),uri(uri),FdClient(FdClient),mutableConfig(MConfig),serv(serv),parser(parser)

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
// string CClient::setupChunkedSending(const std::string & filePath)
// {
//     //  std::cout << "Setting up chunked sending for file: " << filePath << std::endl;
   
//     //     char buffer[this->chunkSize + 1];
//     //     ssize_t bytesRead = read(this->fileFd, buffer, this->chunkSize);
//     //     if (bytesRead == -1) {
//     //         cerr << "Error reading file for chunked sending!" << endl;
//     //         close(this->fileFd);
//     //         return GenerateResErr(500);
//     //     } else if (bytesRead == 0) {
//     //         // End of file reached, send final chunk
//     //         this->response += "0\r\n\r\n";
//     //         close(this->fileFd);
//     //         this->chunkedSending = false; // Finished sending
//     //     } else {
//     //         buffer[bytesRead] = '\0';
//     //         std::ostringstream oss;
//     //         oss << std::hex << bytesRead << "\r\n"; // Chunk size in hex
//     //         oss << std::string(buffer, bytesRead) << "\r\n"; // Chunk data
//     //         this->response += oss.str();
//     //         this->bytesSent += bytesRead;
//     //     }
//     std::cout<<"9999999999999999999999999\n";
//     ifstream file(filePath.c_str(),std::ios::in | std::ios::binary);

//     stringstream buffer;
//     buffer << file.rdbuf();
//     file.close();
//     ostringstream response;
//     // response<<"HTTP/1.1 200 OK \r\n";
//     // response<<"Content-type: "<<"image/jpeg\r\n";
//     response<<"Content-length: "<<buffer.str().size()<<"\r\n\r\n";
//     response<<buffer.str();

//     return response.str();
// }

string CClient::HandleAllMethod()
{

   if(this->NameMethod == "GET")
   {
        
         Get _MGet (*this);
         try {
              return (_MGet.MethodGet()  );
         } catch (const std::runtime_error& e) {
              std::string errMsg = e.what();
              if (errMsg.find("405") != std::string::npos) {
                return GenerateResErr(405);
              } else if (errMsg.find("403") != std::string::npos) {
                return GenerateResErr(403);
              } else {
                return GenerateResErr(500); // Generic server error for other exceptions
              }
         }
   }
   else if(this->NameMethod == "POST")
   {}
    else if(this->NameMethod == "DELETE")
    {}
    else
        return GenerateResErr(405); // Method Not Allowed
    return string();
}



