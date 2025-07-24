#ifndef DELETE_HPP
# define DELETE_HPP


#include "parsing_request.hpp"

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
        bool PerformDelete(const std::string& uri);
};





#endif