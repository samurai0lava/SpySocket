#include "../../inc/webserv.hpp"

void handle_sigint(int signum)
{
    if(signum == SIGINT)
    {
        //handle sigint for server epooll and server.cpp (lokk at ./valgrind_out.log)
    }
}

void handle_signal(void)
{
    if (signal(SIGINT, handle_sigint) == SIG_ERR)
    {
        access_error(500 ,"Internal Server Error");
        logError(500, "Internal Server Error");
        return ;
    }
}