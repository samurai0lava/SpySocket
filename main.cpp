#include "inc/webserv.hpp"

// int main(int argc, char** argv)
// {

//     Config* config = new Config();
//     try
//     {
//         config->StartToSet(parseArgv(argc, argv));
//         // config->printCluster();
//         Servers serv;
//         getServersFds(config, serv);
//         epollFds(serv);

//         // for (vector<int>::iterator it = serv.serversFd.begin(); it != serv.serversFd.end(); it++)
//         //     cout << *it << endl;
//     }
//     catch (std::exception& e)
//     {
//         std::cout << e.what() << std::endl;
//         delete config;
//         return (EXIT_FAILURE);
//     }
//     delete config;
// }
