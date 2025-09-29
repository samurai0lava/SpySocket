#include "../../include/server.hpp"
#include <signal.h>


void handle_sigint(int sig)
{
    (void)sig; // Unused parameter
    std::cout << "\nShutting down server gracefully..." << std::endl;
    Servers* serv = Servers::getInstance();
    serv->setIsRunning(false);
}



void handle_signal() {
    if (signal(SIGINT, handle_sigint) == SIG_ERR) {
        std::cerr << "Error setting up signal handler" << std::endl;
    }
}