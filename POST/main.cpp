#include "../include/methods.hpp"
#include "../inc/webserv.hpp"

// THAT'S A PRETTY BASIC PARSING ASSUMING NOTHING IN THE REQUEST IS MALFORMED

void fillFields(Post &request)
{
    request.contentType = request.header["Content-Type"];
    request.contentLength = request.header["Content-Length"];
    request.returnType = request.header["Accept"];
}

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


string findClosest(vector<string> &locations, string &path)
{
    int bestIndex = -1;
    size_t bestLength = 0;

    for (size_t i = 0; i < locations.size(); ++i)
    {
        string loc = locations[i];
        // cout << "Location : " << loc << endl;

        if (path.compare(0, loc.length(), loc) == 0)
        {
            // cout << loc << " is available in " << path << endl;

            // Make sure the next char is '/' or nothing (to avoid /cgi-bin matching /cgi-binary)

            // cout << "Path length : " << path.length() << " Location length : " << loc.length() << endl;
            
            if (path.length() == loc.length() || path[loc.length()] == '/' || loc[loc.length() - 1] == '/')
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
    {
        //needs modifications later i'll just print the response
        cout << "No matching location found!! 404\n";
        return "null";
    }

    return locations.at(bestIndex);
}


string trimLocation(string path, string foundLocation)
{
    return path.substr(foundLocation.length());
}

void routingFunc(Post &req, Config *config)
{
    auto configMap = config->_cluster;
    vector<string> locations;
    for (auto it = configMap.begin(); it != configMap.end(); it++)
    {
        for (auto i = (*it).second.location.begin(); i != (*it).second.location.end(); i++)
        {
            locations.push_back((*i).first);
        }
    }

    string closestLocation = findClosest(locations, req.path);
    //needs modification later
    if(closestLocation == "null")
        return;
    cout << closestLocation << endl;
    
    for(auto it = (*config->_cluster.begin()).second.location.begin(); it != (*config->_cluster.begin()).second.location.end(); it++)
    {
        if((*it).first == closestLocation)
        {
            if((*it).second.allowedMethods.find("POST") == (*it).second.allowedMethods.end())
                cout << req.notFound() << endl; //response 404 should be returned later
            else
            {
                //check the size of the request body if it matches client_body_size_max in conf file if no return 413 ig if yes proceed with it
                //
            }
        }
    }
}

void handleMethod(int fd, ParsingRequest* parser)
{
    cout << "--> Sent FD : " << fd << endl;
    cout << "************\n";
    for(auto it = parser->getStartLine().begin(); it != parser->getStartLine().end(); it++)
        cout << (*it).first << endl;
}

int main(int argc, char **argv)
{
    Config *config = new Config();
    config->StartToSet(parseArgv(argc, argv));

    Servers serv;
    getServersFds(config, serv);
    epollFds(serv);

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