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
   else if(this->NameMethod == "DELETE")
   {
        DeleteMethode MDelete;
        return (MDelete.PerformDelete(this->uri, this->mutableConfig));
   }
  //  else if(this->NameMethod == "POST")
  //  {}
    else
        return GenerateResErr(405); // Method Not Allowed
    return string();
}



