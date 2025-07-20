#include "../include/server.hpp"

void getServersFds(Config *configFile)
{
    Servers serv;
    serv.configStruct = configFile->_cluster;
    int serversCount = serv.configStruct.size();

    int serverFd;

    for(std::map<std::string, ConfigStruct>::iterator it = serv.configStruct.begin(); it != serv.configStruct.end(); it++)
    {
        serverFd = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in sockStruct;
        sockStruct.sin_family = AF_INET;
        sockStruct.sin_addr.s_addr = INADDR_ANY;
        sockStruct.sin_port = htons((*it).second.listen); //check if the port is valid
        
        serv.serversFd.push_back(serverFd);

        bind(serverFd, (sockaddr *)&sockStruct, sizeof(sockStruct));
    }
}

int main(int argc, char **argv)
{

    Config *config = new Config();
    try
	{
		config->StartToSet(parseArgv(argc, argv));
		// config->printCluster();
        Servers serv;

        for(vector<int>::iterator it = serv.serversFd.begin(); it != serv.serversFd.end(); it++)
            cout << *it << endl;

	}
	catch (std::exception &e)
	{
			std::cout << e.what() << std::endl;
			delete config;
			return (EXIT_FAILURE);
	}
	delete config;

}