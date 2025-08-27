#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

string url_encode(const string& value) {
    ostringstream escaped;
    escaped.fill('0');
    escaped << hex;

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << uppercase;
        escaped << '%' << setw(2) << int((unsigned char)c);
        escaped << nouppercase;
    }

    return escaped.str();
}

// /img/..%2Fsecret.txt
int parse_hex(const std::string& s)
{
    std::istringstream iss(s);
    int n;
    iss >> std::uppercase >> std::hex >> n;
    return n;
}

std::string url_Decode(const std::string& s)
{
    std::string result;
    result.reserve(s.size());
    for (std::size_t i = 0; i < s.size();) {
        if (s[i] != '%') {
            result.push_back(s[i]);
            ++i;
        }
        else {
            result.push_back(parse_hex(s.substr(i + 1, 2)));
            i += 3;
        }
    }
    return result;
}

std::string normalizePath(const std::string& path)
{
    std::vector<std::string> stack;
    std::istringstream iss(path);
    std::string token;

    while (std::getline(iss, token, '/')) {
        if (token.empty() || token == ".") {
            continue;
        }
        if (token == "..") {
            if (!stack.empty()) {
                stack.pop_back();
            }
        }
        else {
            stack.push_back(token);
        }
    }

    std::string normalized = "/";
    for (size_t i = 0; i < stack.size(); ++i) {
        normalized += stack[i];
        if (i + 1 < stack.size()) {
            normalized += "/";
        }
    }
    return normalized;
}



int main(int ac, char** av)
{
    string uri;
    string encoded_uri;
    
    uri = av[1];
        // encoded_uri = url_encode(uri);
        encoded_uri = url_Decode(uri);
        string safe_path = normalizePath(encoded_uri);
        printf("Encoded Uri:%s\n", encoded_uri.c_str());
        printf("Safe Uri:%s\n", safe_path.c_str());
    return 0;
}