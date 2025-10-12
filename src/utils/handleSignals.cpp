#include "../../include/server.hpp"


void handle_sigint(int sig)
{
    (void)sig;
    std::cout << "\n╔════════════════════════════════════╗" << std::endl;
    std::cout << "║   🌐 SpySocket is shutting down... ║" << std::endl;
    std::cout << "╚════════════════════════════════════╝" << std::endl;
    std::cout << "   ↳ Active sessions closed" << std::endl;
    std::cout << "   ↳ Resources freed" << std::endl;
    Servers* serv = Servers::getInstance();
    serv->setIsRunning(false);
    std::cout << "   ✅ Shutdown complete. Bye!" << std::endl;
}



void handle_signal()
{
    if (signal(SIGINT, handle_sigint) == SIG_ERR) {
        std::cerr << "Error setting up signal handler" << std::endl;
    }
}