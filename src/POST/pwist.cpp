#include "../../inc/POST.hpp"

std::string generate_filename()
{
    std::string result;
    static int counter = 0;
    result = "upload_";
    result += (char)(counter + '0');
    result += ".txt";
    counter++;
    return result;
}



string handle_upload(const LocationStruct& location, ParsingRequest& parser)
{
    if (location.upload_enabled == false)
    {
        // 403 Forbidden
        std::cerr << "Error: Upload is not enabled for this location." << std::endl;
        return "HTTP/1.1 403 Forbidden\r\n\r\n";
    }
    if (location.upload_path.empty())
    {
        location.upload_path = location.root;
    }
    std::string line;

    // std::cout << location.root << std::endl;

    std::string boundary = parser.getHeaders().at("boundary");
    cout << "Boundary : " << boundary << endl;
    std::string request = parser.getBody();
    cout << "Body : " << request << endl;

    size_t body_start = request.find("--" + boundary);
    if (body_start == std::string::npos)
    {
        //400 bad request
        std::cerr << "Error: Could not find body start." << std::endl;
        // return;
    }
    size_t body_end = request.find("--" + boundary + "--", body_start);
    if (body_end == std::string::npos)
    {
        //400 bad request
        std::cerr << "Error: Could not find body end." << std::endl;
        // return;
    }
    // should be +4 for "\r\n but since we only have "\n in a string we'll +3 here"
    std::string body = request.substr(body_start + boundary.length() + 4, body_end - body_start - boundary.length() - 4);

    size_t fn_pos = body.find("filename=\"");
    if (fn_pos == std::string::npos)
    {
        //no error maybe it's just a text not a file (username e.g)
        std::cerr << "Error: No filename found.\n";
        // return;
    }
    fn_pos += 10;
    size_t fn_end = body.find('"', fn_pos);
    if (fn_end == std::string::npos)
    {
        //400 bad request
        std::cerr << "Error: Malformed filename.\n";
        // return;
    }
    std::string filename = body.substr(fn_pos, fn_end - fn_pos);
    if (filename.empty())
        filename = generate_filename();
    

    // don't forget it's \r\n in real requests now we only working with \n\n
    size_t content_start = body.find("\r\n\r\n");
    if (content_start == std::string::npos)
    {
        //400 bad request (need to check if an upload request can be bodyless)
        std::cerr << "Error: Could not find content start." << std::endl;
        // return;
    }
    content_start += 4; // Skip past the "\r\n\r\n"

    struct stat st;
    if (stat(location.upload_path.c_str(), &st) == 0)
    {
        // it's a directory
        if (S_ISDIR(st.st_mode)) 
        {
            filename = location.upload_path + "/" + filename;
        }
        else 
        {
            filename = location.upload_path;
        }
    }
    //else some error occured with stat

    std::fstream file(filename.c_str(), std::ios::out);
    if (!file)
    {
        //500 internal
        std::cerr << "Error: Could not open file " << filename << std::endl;
        // return;
    }

    file.write(body.c_str() + content_start, body.size() - content_start);
    if (!file)
    {
        //internal server error
        std::cerr << "Error: Could not write to file " << filename << std::endl;
        // return;
    }
    file.close();
    return "";
}

string	postMethod(string uri, ConfigStruct config,
    ParsingRequest& parser)
{
    string response = "";
    try
    {
        std::pair<std::string, LocationStruct> location = get_location(uri,
            config);

        if (parser.getHeaders()["content-type-value"] == "multipart/form-data")
            response = handle_upload(location.second, parser);

        // cout << location.first << endl;
    }
    catch (exception& e)
    {
    }
    return response;
}