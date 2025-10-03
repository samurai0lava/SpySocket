#include "../inc/webserv.hpp"
// #include "../inc/CClient.h"

void handleMethod(int client_fd, ParsingRequest* parser, const ConfigStruct& config, CClient& client_data)
{
    std::string method = parser->getStartLine()["method"];
    std::string uri = parser->getStartLine()["uri"];
    // std::cout<<"Method : "<< method << std::endl;
    ConfigStruct& mutableConfig = const_cast<ConfigStruct&>(config);
    Servers* serv = Servers::getInstance();

    // Only create new CClient if it's not already initialized (prevents overwriting CGI state)
    if (!client_data.intialized) {
        client_data = CClient(method, uri, client_fd, mutableConfig, serv, parser);
        client_data.SendHeader = false;
        client_data.intialized = true;

        // Check if this is a CGI request and set up CGI handler immediately
        if (!client_data.cgi_handler) {
            client_data.cgi_handler = new CGI();
        }
        if (client_data.cgi_handler->check_is_cgi(*parser)) {
            client_data.is_cgi_request = true;
            std::cout << "[METHODHANDLER DEBUG] CGI request detected, setting up CGI handler" << std::endl;

            std::map<std::string, std::string> env_vars;
            if (client_data.cgi_handler->set_env_var(env_vars, *parser)) {
                bool success = false;
                if (method == "POST") {
                    success = client_data.cgi_handler->execute_with_body(env_vars, parser->getBody());
                }
                else {
                    success = client_data.cgi_handler->execute(env_vars);
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
    // client_data.printInfo();
    return;
}