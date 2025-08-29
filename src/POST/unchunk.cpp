#include "../../inc/webserv.hpp"

int	hex_to_dec(string hex)
{
    int		res = 0;
    string	hexa = "0123456789abcdef";
    int		j = 0;

    if (hex.empty())
        return (-1);
    for (int i = hex.length() - 1; i >= 0; i--)
    {
        if (hex[i] == ' ' || hex[i] == '\t')
            continue;
        if(hexa.find(hex[i]) == string::npos)
            return -1;
        res += hexa.find(tolower(hex[i])) * pow(16, j++);
    }
    return (res);
}

string	unchunk_data(char* chunk, size_t chunk_size)
{

    cout << "************\n";
    cout << chunk;
    cout << "******END******\n";

    int				new_request;
    size_t			body_start;
    int				x;
    static string	filename;
    size_t			pos;
    fstream			file;
    size_t			chars_to_read;
    size_t			eol;
    string			size_hex;
    static bool chunked;
    static int img;

    string stringged_chunk(chunk, chunk_size);
    new_request = 0;
    body_start = 0;
    x = 0;
    // STEP 1: detect if this is a new HTTP request
    if (stringged_chunk.substr(0,
        stringged_chunk.find("\r\n")).find("HTTP") != string::npos)
    {
        pos = stringged_chunk.find("\r\n\r\n");
        if (pos == string::npos)
        {
            cerr << "Malformed request: missing header terminator\n";
            return ("");
        }
        if (stringged_chunk.find("Transfer-Encoding: chunked") != string::npos)
            chunked = true;
        else
            chunked = false;
        if (stringged_chunk.find("Content-Type: image/png") != string::npos)
        {
            img = 1;
            // filename = generate_filename("image_", ".png");
            // cout << "IMAGE FILENAME : " << filename << endl;
        }
        else
        {
            img = 0;
        }
        new_request = 1;
        body_start = pos;
        x = 4; // skip \r\n\r\n

        if(img)
            filename = generate_filename("image_", ".png");
        else if (chunked)
            filename = generate_filename("chunk_", "");
        else
            filename = generate_filename("normal_", "");
    }
    if (body_start + x > stringged_chunk.size())
    {
        cerr << "Body start past end of data\n";
        return ("");
    }
    // STEP 2: isolate just the body
    stringged_chunk = stringged_chunk.substr(body_start + x);
    chunk_size -= body_start + x;
    // STEP 3: prepare output file
    // cout << "NEW REQUEST : " << new_request << endl;
    // cout << "--> " << filename << endl;
    if (new_request)
        file.open(filename.c_str(), ios::out | ios::binary); // new file
    else
        file.open(filename.c_str(), ios::binary | ios::out | ios::app);
    // append
    if (!chunked)
    {
        // Handle unchunked data
        file.write(stringged_chunk.data(), chunk_size);
        // return filename;
    }
    else
    {
        chars_to_read = 0;
        // STEP 4: process chunks
        while (chunk_size > 0)
        {
            // (a) find the CRLF that ends the size line
            eol = stringged_chunk.find("\r\n");
            if (eol == string::npos)
            {
                cerr << "Malformed chunk: missing CRLF after size\n";
                return ("");
            }
            // (b) extract hex size and convert
            size_hex = stringged_chunk.substr(0, eol);
            cout << "HEX : " << size_hex << endl;
            chars_to_read = hex_to_dec(size_hex);
            // (c) advance past the size line + CRLF
            stringged_chunk = stringged_chunk.substr(eol + 2);
            chunk_size -= (eol + 2);
            if (chars_to_read == 0)
            {
                // this is the terminating "0\r\n\r\n"
                cout << "End of data\n";
                break;
            }
            // (d) check we actually have enough bytes left
            cout << "CHARS TO READ : " << chars_to_read << endl;
            cout << "CHUNK SIZE : " << chunk_size << endl;
            if (chars_to_read + 2 > chunk_size)
            { // +2 for the trailing CRLF
                cerr << "Malformed chunk: data shorter than declared size\n";
                return ("");
            }
            // (e) write exactly `chars_to_read` bytes
            file.write(stringged_chunk.data(), chars_to_read);
            // (f) advance past the data + trailing CRLF
            stringged_chunk = stringged_chunk.substr(chars_to_read + 2);
            chunk_size -= (chars_to_read + 2);
        }
    }
    return (filename);
}

