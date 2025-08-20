#include "../../inc/webserv.hpp"

void lower(string &str)
{
    for(int i = 0; i < (int)str.length(); i++)
        str[i] = tolower(str[i]);
}

int hex_to_dec(string hex)
{
    lower(hex);
    // cout << hex << endl;
    int res = 0;
    string hexa = "0123456789abcdef";
    int j = 0;
    for(int i = hex.length() - 1; i >= 0; i--)
    {
        res += hexa.find(hex[i]) * pow(16, j++); 
    }
        
    return res;
}

void unchunk_content(char *buffer)
{
    // cout << "xxxxxxxxxxxxxxxxxxxxxxxxx\n";
    // cout << buffer;
    // cout << "xxxxxxxxxxxxxxxxxxxxxxxxx\n";

    //look for the /r/n 
}