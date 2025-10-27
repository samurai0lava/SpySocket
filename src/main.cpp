#include "../inc/webserv.hpp"

int main(int argc, char** argv)
{
    Config* config = new Config();

    try
    {
        std::string configPath = parseArgv(argc, argv);
        config->StartToSet(configPath);
        Servers* serv = Servers::getInstance();
        handle_signal();
        serv->getServersFds(config, *serv);
        serv->epollFds(*serv);
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
        delete config;
        Servers::destroyInstance();
        return (EXIT_FAILURE);
    }
    delete config;
    Servers::destroyInstance();
}
