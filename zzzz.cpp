#include "../../inc/webserv.hpp"



void lower(string& str)

{

    for (int i = 0; i < (int)str.length(); i++)

        str[i] = tolower(str[i]);

}



int hex_to_dec(const string& hex)

{

    int res = 0;

    string hexa = "0123456789abcdef";

    int j = 0;

    for (int i = hex.length() - 1; i >= 0; i--)

    {

        char c = hex[i];

        if (c == ' ' || c == '\t')

            continue;

        size_t idx = hexa.find(tolower(c));

        if (idx == string::npos)

            return 0; // Invalid hex character

        res += idx * pow(16, j++);

    }

    return res;

}



string generate_filename1(const string& type, const string& termination)

{

    static int counter = 0;

    string result = type + static_cast<char>(counter + '0');

    if (!termination.empty())

        result += termination;

    counter++;

    return result;

}



string unchunk_data(char* chunk, size_t chunk_size)

{

    string stringged_chunk(chunk, chunk_size);



    // Detect new HTTP request

    size_t header_end = stringged_chunk.find("\r\n\r\n");

    if (header_end == string::npos)

    {

        cerr << "Malformed request: missing header terminator\n";

        return "";

    }



    bool chunked = (stringged_chunk.find("Transfer-Encoding: chunked") != string::npos);

    bool img = (!chunked && stringged_chunk.find("Content-Type: image/png") != string::npos);



    string filename;

    if (chunked)

        filename = generate_filename1("chunk_", "");

    else if (img)

        filename = generate_filename1("image_", ".png");

    else

        filename = generate_filename1("normal_", "");



    // Isolate body

    stringged_chunk = stringged_chunk.substr(header_end + 4);

    chunk_size = stringged_chunk.size();



    fstream file;

    if (chunked)

        file.open(filename.c_str(), ios::out | ios::binary);

    else

        file.open(filename.c_str(), ios::out | ios::binary);



    if (!file)

    {

        cerr << "Internal error: cannot open file\n";

        return "";

    }



    if (!chunked)

    {

        file.write(stringged_chunk.data(), chunk_size);

        file.close();

        return filename;

    }



    // Chunked transfer decoding

    while (true)

    {

        size_t eol = stringged_chunk.find("\r\n");

        if (eol == string::npos)

        {

            cerr << "Malformed chunk: missing CRLF after size\n";

            file.close();

            return "";

        }

        string size_hex = stringged_chunk.substr(0, eol);

        int chars_to_read = hex_to_dec(size_hex);

        stringged_chunk = stringged_chunk.substr(eol + 2);



        if (chars_to_read == 0)

        {

            // Last chunk

            file.close();

            break;

        }



        if ((size_t)chars_to_read > stringged_chunk.size())

        {

            cerr << "Malformed chunk: data shorter than declared size\n";

            file.close();

            return "";

        }



        file.write(stringged_chunk.data(), chars_to_read);



        // Advance past chunk data and trailing CRLF

        if (stringged_chunk.size() < (size_t)(chars_to_read + 2) ||

            stringged_chunk.substr(chars_to_read, 2) != "\r\n")

        {

            cerr << "Malformed chunk: missing CRLF after data\n";

            file.close();

            return "";

        }

        stringged_chunk = stringged_chunk.substr(chars_to_read + 2);

    }



    return filename;

}



string unchunked_data(char* data, size_t data_size)

{

    string body(data, data_size);



    size_t header_end = body.find("\r\n\r\n");

    if (header_end == string::npos)

    {

        cerr << "Malformed request: missing header terminator\n";

        return "";

    }



    bool img = (body.find("Content-Type: image/png") != string::npos);



    string filename;

    if (img)

        filename = generate_filename("image_", ".png");

    else

        filename = generate_filename("normal_", "");



    body = body.substr(header_end + 4);



    fstream file(filename.c_str(), ios::out | ios::binary);

    if (!file)

    {

        cerr << "Internal error: cannot open file\n";

        return "";

    }

    file.write(body.data(), body.size());

    file.close();

    return filename;
}