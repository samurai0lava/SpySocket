#include "../include/methods.hpp"
#include "../inc/webserv.hpp"


std::vector<std::string> split(const std::string &str, char delimiter)
{
    std::vector<std::string> result;
    std::string current;

    for (std::string::size_type i = 0; i < str.length(); ++i)
    {
        if (str[i] == delimiter)
        {
            result.push_back(current);
            current = "";
        }
        else
        {
            current += str[i];
        }
    }

    result.push_back(current); // last token
    return result;
}


bool matchedHost(ParsingRequest *parser, Config *conf)
{
    string host = parser->getHeaders()["host"];
    if(conf->_cluster.find(host) != conf->_cluster.end())
    return true;
    return false;
}

string getClosest(vector<string> locationsName, string uri)
{
    int bestIndex = -1;
    size_t bestLength = 0;

    for (size_t i = 0; i < locationsName.size(); ++i)
    {
        string loc = locationsName[i];

        if (uri.compare(0, loc.length(), loc) == 0)
        {
            if (uri.length() == loc.length() || uri[loc.length()] == '/' || loc[loc.length() - 1] == '/')
            {
                if (loc.length() > bestLength)
                {
                    bestIndex = i; //0
                    bestLength = loc.length(); //1
                }
            }
        }
    }
    if(bestIndex == -1)
        return "No matching loc found!";
    else
        return locationsName.at(bestIndex);
}

// bool matchedLocation(string uri, Config *conf)
// {

//     if(getClosest(locations, uri) == "No matching loc found!")
//         return false;
//     return true;
// }

string redirection(string red)
{
    std::string status = red.substr(0, red.find(' '));
    std::string newLoc = red.substr(red.find(' ') + 1);

    std::string statusMsg;
    if (status == "301") statusMsg = "Moved Permanently";
    else if (status == "302") statusMsg = "Found";
    else if (status == "307") statusMsg = "Temporary Redirect";
    else if (status == "308") statusMsg = "Permanent Redirect";
    else statusMsg = "Redirect"; // fallback

    return "HTTP/1.1 " + status + " " + statusMsg + "\r\n"
       "Location: " + newLoc + "\r\n"
       "Content-Length: 0\r\n"
       "\r\n";
}

string methodNotAllowed(std::pair<std::string,LocationStruct> location)
{
    std::string res = "HTTP/1.1 405 Method Not Allowed\r\nAllow: ";
    
    for (auto it = location.second.allowedMethods.begin(); it != location.second.allowedMethods.end(); ++it) {
        res += *it;
        if (std::next(it) != location.second.allowedMethods.end()) {
            res += ", ";
        }
    }
    
    std::string body = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
    res += "\r\nContent-Type: text/html\r\n";
    res += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    res += "\r\n";
    res += body;
    
    return res;
}

string forbidden()
{
    return 
    "HTTP/1.1 403 Forbidden\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: 58\r\n"
    "\r\n"
    "<html><body><h1>403 Forbidden</h1></body></html>";
}

string internalError()
{
    return "HTTP/1.1 500 Internal Server Error\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: 66\r\n"
    "\r\n"
    "<html><body><h1>500 Internal Server Error</h1></body></html>";
}

// string handlePost(ParsingRequest *parser, int clientFd, Config *conf)
// {
//     //TODO: back to that later with multiple servers
//     // if(!matchedHost(parser, conf))
//     //     cerr << "400 Bad request no matching server for that host\n";

    
//     vector<string> locations;
//     //WE ASSUMING WE ONLY HAVE ONE SERVER NOW
//     for(auto it = conf->_cluster.begin(); it != conf->_cluster.end(); it++)
//     {
//         for(auto i = (*it).second.location.begin(); i != (*it).second.location.end(); i++)
//             locations.push_back((*i).first);
//     }

//     map<string, string> startLine = parser->getStartLine();
//     string URI = startLine["uri"];
//     string matchedLoc = getClosest(locations, URI);
//     if(matchedLoc == "No matching loc found!")
//     {
//         return Error::notFound();
//         // return;
//     }
    
//     //GET THE WANTED LOCATION

//     std::pair<std::string,LocationStruct> location;

//     for(auto it = conf->_cluster.begin(); it != conf->_cluster.end(); it++)
//     {
//         for(auto i = (*it).second.location.begin(); i != (*it).second.location.end(); i++)
//         {
//             if((*i).first == matchedLoc)
//             {
//                 location = (*i);
//                 break;
//             }
//         }
//     }


//     //REDIRECTION INCOMPLETE OFC

//     string redirect = location.second._return;
//     if(redirect != "")
//     {
//         return redirection(redirect);
//     }

//     //
//     if(location.second.allowedMethods.find("POST") == location.second.allowedMethods.end())
//     {
//         return methodNotAllowed(location);
//     }

//     //CHECK THE CONTENT LENGTH INSIDE THE GIVEN BLOCK (i don't have a content length variable from the config file TODO : ask zahira)

//     //WORK ON UPLOAD HERE :
//     location.second.upload_enabled = true; //static shit need to be modified later
//     location.second.upload_path = "/api/login/users";

//     if(!location.second.upload_enabled)
//         return forbidden();
//     else if(location.second.upload_path == "")
//         return internalError();
//     else
//     {
        
//     }
    
//     string static_response = "HTTP/1.1 200 OK\r\n"
//     "Content-Type: text/plain\r\n"
//     "Content-Length: 13\r\n"
//     "Connection: close\r\n"
//     "\r\n"
//     "Hello, World!";
//     return static_response;
// }

string handlePost(ParsingRequest *parser, int fd, Config *conf)
{
    vector<string> locations;
    //WE ASSUMING WE ONLY HAVE ONE SERVER NOW
    for(auto it = conf->_cluster.begin(); it != conf->_cluster.end(); it++)
    {
        for(auto i = (*it).second.location.begin(); i != (*it).second.location.end(); i++)
            locations.push_back((*i).first);
    }

    map<string, string> startLine = parser->getStartLine();
    string URI = startLine["uri"];
    string matchedLoc = getClosest(locations, URI);
    if(matchedLoc == "No matching loc found!")
    {
        return Error::notFound();
    }
    
    //GET THE WANTED LOCATION

    std::pair<std::string,LocationStruct> location;

    for(auto it = conf->_cluster.begin(); it != conf->_cluster.end(); it++)
    {
        for(auto i = (*it).second.location.begin(); i != (*it).second.location.end(); i++)
        {
            if((*i).first == matchedLoc)
            {
                location = (*i);
                break;
            }
        }
    }

    //LETS SAY THAT THE REDIRECTION IS CHECKED IN THE REQUEST
    if(!method_allowed(location, "POST"))
        return methodNotAllowed(location);
    if(!uploadSupported(location))
    {
        
    }
    return "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 13\r\n"
    "Connection: close\r\n"
    "\r\n"
    "Hello, World!";
}

string handleMethod(int fd, ParsingRequest* parser, Config *conf)
{
    // cout << "--> Sent FD : " << fd << endl;
    // cout << "************\n";

    map<string, string> startLine = parser->getStartLine();
    if(startLine["method"] == "POST")
        return handlePost(parser, fd, conf);
    return "";
}

int main(int argc, char **argv)
{
    Config *config = new Config();
    config->StartToSet(parseArgv(argc, argv));

    Servers serv;
    getServersFds(config, serv);
    epollFds(serv, config);

    // string line;
    // fstream requestFile("request.txt", ios::in);
    // Post req;
    // string key, value;
    // size_t pos;
    // if (!requestFile.is_open())
    // {
    //     cerr << "Failed to open file 'request.txt'" << endl;
    //     return 1;
    // }
    // int endOfHead = 0;
    // getline(requestFile, line); // retrieve the first line : POST /api/login HTTP/1.1
    // vector<string> firstLine = split(line, ' ');
    // req.path = firstLine.at(1);

    // while (getline(requestFile, line))
    // {
    //     if (line == "") // keep in mind that getline removes the \n
    //     {
    //         endOfHead = 1;
    //         continue;
    //     }
    //     if (!endOfHead)
    //     {
    //         pos = line.find(':');
    //         key = line.substr(0, pos);
    //         value = line.substr(pos + 2); // move past the ':' and the space

    //         req.header.insert(pair<string, string>{key, value});
    //     }
    //     else
    //         req.body.append(line + "\n");
    // }
    // requestFile.close();
    // fillFields(req);
    // routingFunc(req, config);
    return 0;
}