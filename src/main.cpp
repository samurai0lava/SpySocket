#include "../include/singleserver.hpp"
#include "../include/Config.hpp"

static std::string parseArgv(int argc, char **argv)
{
	std::string defaultConfPath ="file.conf";
	if (argc == 1)
	{
		return (defaultConfPath);
	}
	else if (argc > 2)
	{
		std::cout << "Please use webserv with config file only as follows:" << std::endl << "./webserv <config_filename.conf>"<< std::endl;
		exit(1);
	}
	std::string sArgv = argv[1];
	std::string ending = ".conf";
	if ((argv[1] + sArgv.find_last_of(".")) != ending)
	{
		std::cout << "Please use webserv with config file only as follows:" << std::endl  << "./webserv <config_filename.conf>" << std::endl;
		exit(1);
	}
	return (sArgv);
}
// int main(int argc,char **argv)
// {
//     Config *config = new Config();
//     try
// 	{
// 		config->StartToSet(parseArgv(argc, argv));
// 		config->printCluster();


// 	}
// 	catch (std::exception &e)
// 	{
// 			std::cout << e.what() << std::endl;
// 			delete config;
// 			return (EXIT_FAILURE);
// 	}
// 	delete config;
// }

