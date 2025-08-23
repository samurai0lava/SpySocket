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
	// cout << "'" << hex << "'" << endl;
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

string	unchunk_content(char *buffer)
{
	string buff = buffer;
	static string filename;
    static bool chunked;
	string start_line;
	int new_request = 0;
	istringstream body_stream(buff);
	size_t start_pos = 0;
	getline(body_stream, start_line);
	if (start_line.find("HTTP/1.1") != string::npos)
	{
        start_pos = buff.find("\r\n\r\n");
		filename = generate_filename("chunk_");
        if(buff.find("Transfer-Encoding") != string::npos)
        chunked = true;
        else
        chunked = false;
		new_request = 1;
	}
    
    cout << "CHUNKED : " << chunked << endl;
	if (start_pos != string::npos)
	{
		if (new_request)
			start_pos += 4;
		buffer = buffer + start_pos;
        fstream file;
        string line;
        istringstream body_stream(buff.substr(start_pos));
        getline(body_stream, line);
        if (line.empty())
        {
            cerr << "No body to unchunk\n";
            return "Empty";
        }

        if(chunked == true)
        {
            cout << "xxxxxxxxxxx\n";
            cout << buffer << endl;
            cout << "xxxxxxxxxxx\n";

            int chars_to_read = 0;
            chars_to_read = hex_to_dec(line.substr(0, line.length() - 1));
            cout << "-----> " << line << endl;
            if(new_request)
                file.open(filename.c_str(), ios::out);
            else
                file.open(filename.c_str(), ios::out | ios::app);
            if (!file)
                cerr << "Internal error\n";
            cout << "--> " << chars_to_read << endl;
            while (chars_to_read)
            {
                buffer = buffer + line.length() + 1;
                file.write(buffer, chars_to_read);
                buffer = buffer + chars_to_read;
                buff = buffer;
                if (buff.empty())
                {
                    cout << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n";
                    break ;
                }
                else if(buff.find("\r\n") != string::npos)
                {
                    cout << "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n";
                    cout << buff.find("\r\n") << endl;
                    cout << "-->" << buff << "<--" << endl;
                }
                //mostly we have /r/n before the hex line so check that out (causing me problems)
                chars_to_read = hex_to_dec(buff.substr(0, buff.find("\r\n")));
                
                cout << "CHARS TO READ : " << chars_to_read << endl;
            }
        }
        else
        {
            file.open(filename.c_str(), ios::out);
            if (!file)
                cerr << "Internal error\n";
            // cout << "---> " << buffer << endl;
            // cout << sizeof(buffer) << endl;
            // for(int i = 0; i < content_length; i++)
            for(int i = 0; buffer[i]; i++)
                file.write(&buffer[i], 1);
        }

        file.close();
	}
    return filename;
}