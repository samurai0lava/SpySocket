#include "inc/webserv.hpp"


//test1

int main() {
    ParsingRequest parser;
    
    // Simulate partial HTTP request arriving in chunks
    std::vector<std::string> chunks = {
        "GET /api/users HTTP/1.1\r\n",
        "Host: localhost:8080\r\n",
        "Content-Length: 27\r\n",
        "Content-Type: application/json\r\n",
        "\r\n",
        "{\"name\":\"John\",\"age\":30}"
    };
    
    std::cout << BLUE "=== NGINX-Style Incremental Parsing ===" RESET << std::endl;
    
    for (size_t i = 0; i < chunks.size(); ++i) {
        std::cout << YELLOW "Chunk " << (i + 1) << ": " RESET << "'" << chunks[i] << "'" << std::endl;
        
        ParsingRequest::ParseResult result = parser.feed_data(chunks[i].c_str(), chunks[i].length());
        
        switch (result) {
            case ParsingRequest::PARSE_OK:
                std::cout << GREEN "✅ Parsing complete!" RESET << std::endl;
                break;
            case ParsingRequest::PARSE_AGAIN:
                std::cout << BLUE "🔄 Need more data..." RESET << std::endl;
                break;
            case ParsingRequest::PARSE_ERROR_400:
                std::cout << RED "❌ Parse error (400 Bad Request)" RESET << std::endl;
                return 1;
            case ParsingRequest::PARSE_ERROR_501:
                std::cout << RED "❌ Parse error (501 Not Implemented)" RESET << std::endl;
                return 1;
        }
        
        if (parser.is_complete()) {
            std::cout << GREEN "\n🎉 Request fully parsed!" RESET << std::endl;
            
            // Access parsed data
            std::map<std::string, std::string> start_line = parser.getStartLine();
            std::map<std::string, std::string> headers = parser.getHeaders();
            std::string body = parser.getBody();
            
            std::cout << BLUE "Method: " RESET << start_line["method"] << std::endl;
            std::cout << BLUE "URI: " RESET << start_line["uri"] << std::endl;
            std::cout << BLUE "Version: " RESET << start_line["version"] << std::endl;
            
            std::cout << BLUE "Headers:" RESET << std::endl;
            for (std::map<std::string, std::string>::const_iterator it = headers.begin(); 
                 it != headers.end(); ++it) {
                std::cout << "  " << it->first << ": " << it->second << std::endl;
            }
            
            if (!body.empty()) {
                std::cout << BLUE "Body: " RESET << body << std::endl;
            }
            
            break;
        }
    }
    
    return 0;
}
