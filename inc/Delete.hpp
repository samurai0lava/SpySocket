#ifndef DELETE_HPP
# define DELETE_HPP


#include "parsing_request.hpp"
#include "../include/Struct.hpp"
#include "../include/Config.hpp"

//so what we need to make this Delete Work mmmm
//202 Accepted
//204 No content
//200 OK
// DELETE shouldnt include a body in request
// 409 URI its not a file but URI DOSNT END WITH /
// 403 u dont have the right to delete that dr
// 500 FAILED TO DELETE THAT DIR


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
        int getStatusCode() const { return status_code; }
        std::string getStatusPhrase() const { return status_phrase; }
        bool checkIfAllowed(const std::string& method, const ConfigStruct& config, const std::string& uri) const;
        std::string mapUriToPath(const std::string& uri, const ConfigStruct& config) const;
};





#endif