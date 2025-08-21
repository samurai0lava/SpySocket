#include "../../inc/webserv.hpp"

void	lower(string &str)
{
	for (int i = 0; i < (int)str.length(); i++)
		str[i] = tolower(str[i]);
}

int	hex_to_dec(string hex)
{
	int		res;
	string	hexa;
	int		j;

	lower(hex);
	cout << "'" << hex << "'" << endl;
	res = 0;
	hexa = "0123456789abcdef";
	j = 0;
	for (int i = hex.length() - 1; i >= 0; i--)
	{
		if (hex[i] == ' ' || hex[i] == '\t')
			continue ;
		res += hexa.find(hex[i]) * pow(16, j++);
	}
	return (res);
}

// void	unchunk_content(char *buffer)
// {
// 	string	buff;
// 	size_t	start_pos;
// 	string	filename;
// 	string	ptr;
// 	size_t	line_end;
// 	// size_t	line_len;
// 	int		chunk_size;

// 	cout << "xxxxxxxxxxxxxxxxxxxxxxxxx\n";
// 	cout << buffer;
// 	cout << "xxxxxxxxxxxxxxxxxxxxxxxxx\n";
// 	buff = buffer;
// 	start_pos = buff.find("\r\n\r\n");
// 	if (start_pos == string::npos)
// 	{
// 		// cerr << "No headers/body separator found\n";
// 		return ;
// 	}
// 	start_pos += 4; // Skip headers to body
// 	filename = generate_filename("chunk_");
// 	ofstream file(filename.c_str(), ios::out | ios::binary);
// 	if (!file)
// 	{
// 		cerr << "Failed to open file\n";
// 		return ;
// 	}
// 	ptr = buffer + start_pos;
// 	while (true)
// 	{
// 		// Find next \r\n to get chunk size line
// 		line_end = ptr.find("\r\n");
// 		if (line_end == string::npos)
// 		{
// 			cerr << "Malformed chunked encoding: no CRLF after chunk size\n";
// 			break ;
// 		}
// 		// line_len = line_end - ptr;
// 		// if (line_len == 0)
// 		// {
// 		// 	cerr << "Malformed chunk size line\n";
// 		// 	break ;
// 		// }
// 		// Get chunk size in decimal
// 		chunk_size = hex_to_dec(ptr);
// 		if (chunk_size == 0)
// 		{
// 			// End of chunks
// 			break ;
// 		}
// 		// Move pointer after chunk size line + \r\n
// 		ptr = line_end + 2;
// 		// Write chunk data to file
// 		file.write(ptr.c_str(), chunk_size);
// 		// Move pointer after chunk data
// 		ptr += chunk_size;
// 		// After chunk data there must be \r\n, skip it
// 		if (ptr[0] != '\r' || ptr[1] != '\n')
// 		{
// 			cerr << "Malformed chunk: missing CRLF after data\n";
// 			break ;
// 		}
// 		ptr += 2;
// 	}
// 	file.close();
// }

void unchunk_content(char *buffer)
{
    string buff = buffer;
    // cout << "xxxxxxxxxxxxxxx\n";
    // cout << buff;
    // cout << "xxxxxxxxxxxxxxx\n";
    string filename = generate_filename("chunk_");
    size_t start_pos = buff.find("\r\n\r\n");
    fstream file(filename.c_str(), ios::out);
    if(!file)
        cerr << "Internal error\n";

    if(start_pos != string::npos)
    {
        start_pos += 4;
        buffer = buffer + start_pos;
        string line;
        // int start = 0;
        // int get_count = 0;
        int chars_to_read = 0;
        istringstream body_stream(buff.substr(start_pos));
        // while(getline(body_stream, line))
        // {
        //     line = line.substr(0, line.length() - 1);
        //     cout << "---> " << line << endl;
        //     if(!get_count)
        //     {
        //         chars_to_read = hex_to_dec(line);
        //         cout << "READ SIZE : " << chars_to_read << endl;
        //         if(chars_to_read == 0)
        //             break; //end of chunks
        //         get_count = 1;
        //         continue;
        //     }
        //     cout << "CHARS TO READ : " << chars_to_read << endl;
        //     cout << "LINE LENGTH : " << line.length() << endl;
        //     for(int i = 0; )
        //     file.write(line.c_str(), chars_to_read);
        //     get_count = 0;
        // }
        getline(body_stream, line);
        chars_to_read = hex_to_dec(line.substr(0, line.length() - 1));
        
        while(chars_to_read)
        {
            buffer = buffer + line.length() + 1;
            cout << "xxxxxxxxxxxxxxx\n";
            cout << buffer << endl;
            cout << "xxxxxxxxxxxxxxx\n";
            file.write(buffer, chars_to_read);
            buffer = buffer + chars_to_read;
            cout << "***************\n";
            cout << buffer << endl;
            cout << "***************\n";
            buff = buffer;
            if(buff.empty())
                break;
            chars_to_read = hex_to_dec(buff.substr(0, buff.find("\r\n")));
            cout << "--> " << chars_to_read << endl;
        }
    }
}