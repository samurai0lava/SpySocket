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



void handle_request(int fd, const LocationStruct &location, ParsingRequest &parser)
{
    // std::istringstream request_stream(request);
    std::string line;
    int count = 0;

    std::string boundary = parser.getHeaders().at("boundary");
    cout << "Boundary : " << boundary << endl;
    std::string request = parser.getBody();
    cout << "Body : " << request << endl;

    size_t body_start = request.find("--" + boundary);
    if (body_start == std::string::npos)
    {
        //400 bad request
        std::cerr << "Error: Could not find body start." << std::endl;
        return;
    }
    size_t body_end = request.find("--" + boundary + "--", body_start);
    if (body_end == std::string::npos)
    {
        //400 bad request
        std::cerr << "Error: Could not find body end." << std::endl;
        return;
    }
    // should be +4 for "\r\n but since we only have "\n in a string we'll +3 here"
    std::string body = request.substr(body_start + boundary.length() + 3, body_end - body_start - boundary.length() - 3);

    size_t fn_pos = body.find("filename=\"");
    if (fn_pos == std::string::npos)
    {
        //no error maybe it's just a text not a file (username e.g)
        std::cerr << "Error: No filename found.\n";
        return;
    }
    fn_pos += 10;
    size_t fn_end = body.find('"', fn_pos);
    if (fn_end == std::string::npos)
    {
        //400 bad request
        std::cerr << "Error: Malformed filename.\n";
        return;
    }
    std::string filename = body.substr(fn_pos, fn_end - fn_pos);
    if(filename.empty())
        filename = generate_filename();
    std::fstream file(filename.c_str(), std::ios::out);
    if (!file)
    {
        //500 internal
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    // don't forget it's \r\n in real requests now we only working with \n\n
    size_t content_start = body.find("\n\n");
    if (content_start == std::string::npos)
    {
        //400 bad request (need to check if an upload request can be bodyless)
        std::cerr << "Error: Could not find content start." << std::endl;
        return;
    }
    content_start += 2; // Skip past the "\r\n\r\n"
    file.write(body.c_str() + content_start, body.size() - content_start);
    if (!file)
    {
        //internal server error
        std::cerr << "Error: Could not write to file " << filename << std::endl;
        return;
    }
    file.close();
}

void	postMethod(int fd, string uri, ConfigStruct config,
		ParsingRequest &parser)
{
	try
	{
		std::pair<std::string, LocationStruct> location = get_location(fd, uri,
				config);

		handle_request(fd, location.second, parser);

		// cout << location.first << endl;
	}
	catch (exception &e)
	{
	}
}