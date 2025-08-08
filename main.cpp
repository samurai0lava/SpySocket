#include "inc/webserv.hpp"

int main(int argc, char** argv)
{
    Config* config = new Config();
    std::cout << GREEN "--------Webserv started with config file: " RESET << parseArgv(argc, argv) << std::endl;

    try
    {
        config->StartToSet(parseArgv(argc, argv));
        config->printCluster();
        Servers serv;
        std::cout<<"01 =:: "<<config->getAutoindex()<<std::endl;
        serv.getServersFds(config, serv);
        serv.epollFds(serv);
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
        delete config;
        return (EXIT_FAILURE);
    }
    delete config;
}
