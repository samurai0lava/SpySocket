#include "../include/methods.hpp"

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


//THIS METHOD IS NOT IT (NOT SO ACCURATE) 
// int findClosest(vector<string> locations, string path)
// {
//     int index = -1;
//     int max = INT_MAX;
//     int comp = 0;
//     int near = 0;

//     for(auto it = locations.begin(); it != locations.end(); it++)
//     {
//         index++;
//         comp = abs(strcmp(path.c_str(), (*it).c_str()));
//         cout << "First : " << path << " Second : " << *it << "\n" << "Comparaison result : " << comp << " Max : " << max << endl;
//         if(comp < max)
//         {
//             max = comp;
//             near = index;
//         }
//     }
//     return near;
// }

string findClosest(vector<string> locations, string path)
{
    int matchedChars = 0;
    int closestIndex = 0;
    int comp = 0;
    int temp = -1;
    vector<string> potentialMatches;

    for(auto it = locations.begin(); it != locations.end(); it++)
    {
        if((*it).length() <= path.length())
            potentialMatches.push_back(*it);
    }

    for(auto it = potentialMatches.begin(); it != potentialMatches.end(); it++)
    {
        temp++;
        matchedChars = 0;
        cout << "----> " << *it << endl;
        for(int i = 0; i < path.length(); i++)
        {
            if((*it)[i] != path[i])
                break;
            matchedChars++;
        }
        cout << "Matched count : " << matchedChars << endl;
        cout << "Comp count : " << comp << endl;

        if(matchedChars > comp)
        {
            closestIndex = temp;
            comp = matchedChars;
        }
    }
    return potentialMatches.at(closestIndex);
}   

string trimLocation(string path, string foundLocation)
{
    return path.substr(foundLocation.length());
}

void routingFunc(Post &req, Config *config)
{
    auto configMap = config->_cluster;
    vector<string> locations;
    for(auto it = configMap.begin(); it != configMap.end(); it++)
    {
        for(auto i = (*it).second.location.begin(); i != (*it).second.location.end(); i++)
        {
            locations.push_back((*i).first);
        }
    }
    string index = findClosest(locations, req.path);
    cout << index << endl;
    //we'll trim the path so we'll search for the file in the location
    // string file = trimLocation(req.path, locations.at(index));
    // cout << file << endl;
}

int main(int argc, char **argv)
{
    Config *config = new Config();
    config->StartToSet(parseArgv(argc, argv));

    string line;
    fstream requestFile("request.txt", ios::in);
    Post req;
    string key, value;
    size_t pos;
    if (!requestFile.is_open())
    {
        cerr << "Failed to open file 'request.txt'" << endl;
        return 1;
    }
    int endOfHead = 0;
    getline(requestFile, line); // retrieve the first line : POST /api/login HTTP/1.1
    vector<string> firstLine = split(line, ' ');
    req.path = firstLine.at(1);

    while (getline(requestFile, line))
    {
        if (line == "") // keep in mind that getline removes the \n
        {
            endOfHead = 1;
            continue;
        }
        if (!endOfHead)
        {
            pos = line.find(':');
            key = line.substr(0, pos);
            value = line.substr(pos + 2); // move past the ':' and the space

            req.header.insert(pair<string, string>{key, value});
        }
        else
            req.body.append(line + "\n");
    }
    requestFile.close();
    fillFields(req);
    routingFunc(req, config);
    return 0;
}