#ifndef DELETE_HPP
# define DELETE_HPP


#include "parsing_request.hpp"
#include "../inc/Struct.hpp"
#include "../inc/Config.hpp"

class DeleteMethode : public ParsingRequest
{
    private:


    public:
        DeleteMethode(){}
        ~DeleteMethode(){}
        bool CheckFile(const std::string& uri);
        bool CheckisDir(const std::string& uri);
        bool CheckAccess(const std::string& uri);
        bool checkReqForDelete(ParsingRequest& request);
        std::string PerformDelete(const std::string& uri, const ConfigStruct& config);
        std::string generate_success_resp(void);
        std::string getErrorPageFromConfig(int statusCode, const ConfigStruct& config);
        std::string getStatusMessage(int statusCode);
        int getStatusCode() const { return status_code; }
        std::string getStatusPhrase() const { return status_phrase; }
        bool checkIfAllowed(const std::string& method, const ConfigStruct& config, const std::string& uri) const;
        std::string mapUriToPath(const std::string& uri, const ConfigStruct& config) const;
        bool isDirEmpty(const std::string& path) const;
};



#endif
