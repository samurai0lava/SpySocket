#include "../inc/webserv.hpp"
// #include "../inc/CClient.h"

void handleMethod(int client_fd, ParsingRequest* parser, const ConfigStruct& config, CClient& client_data)
{
    std::string method = parser->getStartLine()["method"];
    std::string uri = parser->getStartLine()["uri"];
    ConfigStruct& mutableConfig = const_cast<ConfigStruct&>(config);
    Servers* serv = Servers::getInstance();
    if (!client_data.intialized) {
        client_data = CClient(method, uri, client_fd, mutableConfig, serv, parser);
        client_data.SendHeader = false;
        client_data.intialized = true;
        if (!client_data.cgi_handler) {
            client_data.cgi_handler = new CGI();
        }
        if (client_data.cgi_handler->check_is_cgi(*parser)) {
            client_data.is_cgi_request = true;

            // Get the location configuration for this request
            std::pair<std::string, LocationStruct> loc_pair = get_location(uri, mutableConfig);
            LocationStruct location = loc_pair.second;

            std::map<std::string, std::string> env_vars;
            if (client_data.cgi_handler->set_env_var(env_vars, *parser)) {
                bool success = false;
                if (method == "POST") {
                    success = client_data.cgi_handler->execute_with_body(env_vars, parser->getBody(), location);
                }
                else {
                    success = client_data.cgi_handler->execute(env_vars, location);
                }

                if (!success) {
                    client_data.cgi_handler->close_cgi();
                    delete client_data.cgi_handler;
                    client_data.cgi_handler = NULL;
                    client_data.is_cgi_request = false;
                }
            }
            else {
                client_data.cgi_handler->close_cgi();
                delete client_data.cgi_handler;
                client_data.cgi_handler = NULL;
                client_data.is_cgi_request = false;
            }
        }
    }
    return;
}