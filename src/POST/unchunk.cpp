#include "../../inc/webserv.hpp"

void	lower(string& str)
{
    for (int i = 0; i < (int)str.length(); i++)
        str[i] = tolower(str[i]);
}

int	hex_to_dec(string hex)
{
    int		res;
    string	hexa;
    int		j;

    if (hex.empty())
        return (0);
    lower(hex);
    // cout << "'" << hex << "'" << endl;
    res = 0;
    hexa = "0123456789abcdef";
    j = 0;
    for (int i = hex.length() - 1; i >= 0; i--)
    {
        if (hex[i] == ' ' || hex[i] == '\t')
            continue;
        res += hexa.find(hex[i]) * pow(16, j++);
    }
    return (res);
}

// string	unchunk_content(char *buffer)
// {
//     // cout << "xxxxxxxxxxx\n";
//     // cout << buffer << endl;
//     // cout << "xxxxxxxxxxx\n";
// 	string buff = buffer;
// 	static string filename;
//     static bool chunked;
// 	string start_line;
// 	int new_request = 0;
// 	istringstream body_stream(buff);
// 	size_t start_pos = 0;
// 	getline(body_stream, start_line);
// 	if (start_line.find("HTTP/1.1") != string::npos)
// 	{
//         start_pos = buff.find("\r\n\r\n");
// 		filename = generate_filename("chunk_");
//         if(buff.find("Transfer-Encoding") != string::npos)
//         chunked = true;
//         else
//         chunked = false;
// 		new_request = 1;
// 	}

//     cout << "CHUNKED : " << chunked << endl;
// 	if (start_pos != string::npos)
// 	{
// 		if (new_request)
// 			start_pos += 4;
// 		buffer = buffer + start_pos;
//         fstream file;
//         string line;
//         istringstream body_stream(buff.substr(start_pos));
//         getline(body_stream, line);
//         if (line.empty())
//         {
//             cerr << "No body to unchunk\n";
//             return ("Empty");
//         }

//         if(chunked == true)
//         {
//             cout << "**************\n";
//             cout << buffer ;
//             cout << "**************\n";

//             int chars_to_read = 0;
//             chars_to_read = hex_to_dec(line.substr(0, line.length() - 1)); //
// -1 for \r since the \n got trimmed by getline
//             cout << "CHARS TO READ 1: " << chars_to_read << endl;
//             if(new_request)
//                 file.open(filename.c_str(), ios::out);
//             else
//                 file.open(filename.c_str(), ios::out | ios::app);
//             if (!file)
//                 cerr << "Internal error\n";
//             while (chars_to_read)
//             {
//                 buffer = buffer + line.length() + 1;
//                 file.write(buffer, chars_to_read);
//                 buffer = buffer + chars_to_read;
//                 buff = buffer;
//                 if (buff.empty())
//                 {
//                     break ;
//                 }
//                 //mostly we have
// / r / n before the hex line so check that out(causing me problems)
//                 chars_to_read = hex_to_dec(buff.substr(0,
// buff.find("\r\n")));
//                 if(buff.find("\r\n") == 0)
//                     chars_to_read = hex_to_dec(buff.substr(0,
// buff.find("\r\n") + 2));
//                 cout << "CHARS TO READ 2: " << chars_to_read << endl;
//             }
//         }
//         else
//         {
//             file.open(filename.c_str(), ios::out);
//             if (!file)
//                 cerr << "Internal error\n";
//             // cout << "---> " << buffer << endl;
//             // cout << sizeof(buffer) << endl;
//             // for(int i = 0; i < content_length; i++)
//             for(int i = 0; buffer[i]; i++)
//                 file.write(&buffer[i], 1);
//         }

//         file.close();
// 	}
//     return (filename);
// }

string	treat_chunked_data(char* chunk, size_t chunk_size)
{
    int				new_request;
    size_t			body_start;
    int				x;
    static string	filename;
    size_t			pos;
    fstream			file;
    size_t			chars_to_read;
    size_t			eol;
    string			size_hex;

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
        new_request = 1;
        body_start = pos;
        x = 4; // skip \r\n\r\n
        filename = generate_filename("chunk_", "");
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
    cout << "NEW REQUEST : " << new_request << endl;
    cout << "--> " << filename << endl;
    if (new_request)
        file.open(filename.c_str(), ios::out | ios::binary); // new file
    else
        file.open(filename.c_str(), ios::binary | ios::out | ios::app);
    // append
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
    return (filename);
}

string	treat_unchunked_data(char* data, size_t data_size)
{
    static string	filename;
    size_t			pos;
    size_t			start_pos;
    fstream file;

    string body(data, data_size);
    if (body.find("Content-Type: image/png"))
    {
        int new_request = 0;
        int body_start = 0;
        int x = 0;
        if (body.substr(0, body.find("\r\n")).find("HTTP") != string::npos)
        {
            pos = body.find("\r\n\r\n");
            if (pos == string::npos)
            {
                cerr << "Malformed request: missing header terminator\n";
                return ("");
            }
            new_request = 1;
            body_start = pos;
            x = 4; // skip \r\n\r\n
            filename = generate_filename("image_", ".png");
        }

        body = body.substr(body_start + x);
        if (new_request)
            file.open(filename.c_str(), ios::out | ios::binary);
        else
            file.open(filename.c_str(), ios::out | ios::binary | ios::app);
        if(!file)
        {
            cerr << "INTERNAL ERROR 500\n";
            return "";
        }
        file.write(body.c_str(), body.length());
    }
    else
    {
        start_pos = body.find("\r\n\r\n");
        if (start_pos == string::npos)
        {
            cerr << "Malformed request no headers end!\n";
            return ("");
        }
        body = body.substr(start_pos + 4);
        filename = generate_filename("normal_", "");
        file.open(filename.c_str(), ios::binary | ios::out);
        if (!file)
        {
            cerr << "Internal error\n";
            return ("");
        }
        file.write(body.c_str(), body.length());
    }
    return (filename);
}

string	unchunk_data(char* chunk, size_t chunk_size)
{
    string	res;
//GOTTA WORK ON THE UNCHUNKED DATA TOO
    // cout << "************\n";
    // cout << chunk;
    // cout << "*****END*****\n";
    string stringged_chunk(chunk, chunk_size);
    // if (stringged_chunk.find("Transfer-Encoding") != string::npos)
    // {
        res = treat_chunked_data(chunk, chunk_size);
    // }
    // else
    // {
    //     res = treat_unchunked_data(chunk, chunk_size);
    // }
    return (res);
}
