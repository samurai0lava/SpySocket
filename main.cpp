// #include "inc/webserv.hpp"
// #include <vector>

// void test_legacy_parsing();
// void test_incremental_parsing();
// void simulate_network_chunks();

// int main() {

// void test_incremental_parsing() {
//     std::cout << BLUE "=== Test 2: Incremental Parsing (NGINX-style) ===" RESET << std::endl;
    
//     // Simulate data arriving in chunks
//     std::vector<std::string> chunks;
//     chunks.push_back("GET /api/data HTTP/1.1\r\n");
//     chunks.push_back("Host: localhost:8080\r\n");
//     chunks.push_back("User-Agent: TestClient/1.0\r\n");
//     chunks.push_back("Accept: application/json\r\n");
//     chunks.push_back("Connection: close\r\n");
//     chunks.push_back("\r\n");
    
//     ParsingRequest parser;
    
//     for (size_t i = 0; i < chunks.size(); ++i) {
//         std::cout << YELLOW "Feeding chunk " << (i + 1) << ": " RESET 
//                   << "'" << chunks[i] << "'" << std::endl;
        
//         ParsingRequest::ParseResult result = parser.feed_data(chunks[i].c_str(), chunks[i].length());
        
//         switch (result) {
//             case ParsingRequest::PARSE_OK:
//                 std::cout << GREEN "✅ Parsing complete!" RESET << std::endl;
//                 break;
//             case ParsingRequest::PARSE_AGAIN:
//                 std::cout << BLUE "🔄 Need more data..." RESET << std::endl;
//                 break;
//             case ParsingRequest::PARSE_ERROR_400:
//                 std::cout << RED "❌ Bad Request (400)" RESET << std::endl;
//                 return;
//             case ParsingRequest::PARSE_ERROR_501:
//                 std::cout << RED "❌ Not Implemented (501)" RESET << std::endl;
//                 return;
//         }
        
//         if (parser.is_complete()) {
//             std::cout << GREEN "🎉 Request fully parsed!" RESET << std::endl;
            
//             std::map<std::string, std::string> start_line = parser.getStartLine();
//             std::map<std::string, std::string> headers = parser.getHeaders();
            
//             std::cout << "Method: " << start_line["method"] << std::endl;
//             std::cout << "URI: " << start_line["uri"] << std::endl;
//             std::cout << "Version: " << start_line["version"] << std::endl;
//             std::cout << "Headers:" << std::endl;
//             for (std::map<std::string, std::string>::const_iterator it = headers.begin(); 
//                  it != headers.end(); ++it) {
//                 std::cout << "  " << it->first << ": " << it->second << std::endl;
//             }
//             break;
//         }
//     }
// }

// void simulate_network_chunks() {
//     std::cout << BLUE "=== Test 3: Realistic Network Simulation ===" RESET << std::endl;
    
//     // Simulate very small chunks (like real network packets)
//     std::string full_request = 
//         "POST /submit HTTP/1.1\r\n"
//         "Host: example.com\r\n"
//         "Content-Length: 13\r\n"
//         "Content-Type: text/plain\r\n"
//         "\r\n"
//         "Hello, World!";
    
//     ParsingRequest parser;
    
//     // Feed data byte by byte (extreme case)
//     std::cout << YELLOW "Feeding data byte by byte..." RESET << std::endl;
    
//     for (size_t i = 0; i < full_request.length(); ++i) {
//         char single_byte = full_request[i];
        
//         ParsingRequest::ParseResult result = parser.feed_data(&single_byte, 1);
        
//         // Only print status every 10 bytes to avoid spam
//         if (i % 10 == 0 || result != ParsingRequest::PARSE_AGAIN) {
//             std::cout << "Byte " << i << ": ";
//             switch (result) {
//                 case ParsingRequest::PARSE_OK:
//                     std::cout << GREEN "Complete!" RESET << std::endl;
//                     break;
//                 case ParsingRequest::PARSE_AGAIN:
//                     std::cout << BLUE "Continue..." RESET << std::endl;
//                     break;
//                 case ParsingRequest::PARSE_ERROR_400:
//                     std::cout << RED "Error 400" RESET << std::endl;
//                     return;
//                 case ParsingRequest::PARSE_ERROR_501:
//                     std::cout << RED "Error 501" RESET << std::endl;
//                     return;
//             }
//         }
        
//         if (parser.is_complete()) {
//             std::cout << GREEN "🎉 Byte-by-byte parsing successful!" RESET << std::endl;
            
//             std::map<std::string, std::string> start_line = parser.getStartLine();
//             std::string body = parser.getBody();
            
//             std::cout << "Method: " << start_line["method"] << std::endl;
//             std::cout << "URI: " << start_line["uri"] << std::endl;
//             std::cout << "Body: " << body << std::endl;
//             std::cout << "Total bytes processed: " << (i + 1) << std::endl;
//             break;
//         }
//     }
// }
