#include "../inc/Config.hpp"

// #include "Config.hpp"


std::string parseArgv(int argc, char** argv)
{
	if (argc > 2 || argc < 2)
	{
		std::cout << RED "Please use webserv with config file only as follows:" << std::endl << "./webserv <config_filename.conf>"   RESET<< std::endl;
		throw std::runtime_error("Invalid number of arguments");
	}
	std::string sArgv = argv[1];
	std::string ending = ".conf";
	if ((argv[1] + sArgv.find_last_of(".")) != ending)
	{
		std::cout << "Please use webserv with config file only as follows:" << std::endl << "./webserv <config_filename.conf>" << std::endl;
		throw std::runtime_error("Invalid config file format");
	}
	return (sArgv);
}

void Config::_parseServerBlock(std::string serverBlock)
{
	std::string server;
	std::string buffer;
	std::stringstream serverStream;
	serverStream << serverBlock;
	while(serverStream.good())
	{
		buffer.clear();
		std::getline(serverStream, buffer);
		if (buffer.length() == 0)
			continue ;
		size_t start = buffer.find_first_not_of("\n\r\t\f\v ");
		if (start == std::string::npos)
			continue ;
		size_t end = buffer.find_first_of(";");
		if (end == std::string::npos)
		{
			end = buffer.find_last_not_of("\n\r\t\f\v ");
			buffer = buffer.substr(start, (end - start + 1));
		}
		else
		{
			--end;
			buffer = buffer.substr(start, (end - start + 1));
			end = buffer.find_last_not_of("\n\r\t\f\v ");
			buffer = buffer.substr(0, end + 1);
		}
		if (buffer.length() > 0)
		{
			server.append(buffer, 0, buffer.length());
			server.append("\n", 0, 1);

		}
	}
	this->_createConfigStruct(server);
}

void Config::_createConfigStruct(std::string server)
{
	ConfigStruct tmp;
	tmp.serverName = "";
	tmp.root = "";
	// tmp.autoIndex = false;
	if (server.find("server_name") == std::string::npos)
		throw std::runtime_error("Missing server_name in server configuration");
	std::string serverName = server.substr(server.find("server_name"));
	serverName = serverName.substr(serverName.find_first_of(" ") + 1);
	serverName = serverName.substr(0, serverName.find_first_of("\n"));
	ConfigStruct confStruct = tmp;
	SingleServerConfig temp(server, &confStruct);
	this->_cluster.insert(std::make_pair(serverName, confStruct));

}

Config::Config()
{
}
Config::~Config()
{

}

void Config::StartToSet(std::string configPath)
{
	std::stringstream streamBuffer;

	this->_configPath = configPath;
	this->_configFile.open(this->_configPath.c_str());
	if (!this->_configFile.is_open())
	{
		throw Config::FileOpenException();
	}
	streamBuffer << this->_configFile.rdbuf();
	this->_configFile.close();
	std::string buffer = streamBuffer.str();
	this->_checkBrackets(buffer);
	this->_checkRedirectionLoops();
	this->_checkDuplicateListenAddresses();
	std::cout <<"Configuration file parsed successfully!"  << std::endl;
	std::cout << GREEN "Starting server..." RESET << std::endl;
	std::cout << "Server Started with config file: " << BLUE << configPath << RESET << std::endl;
	std::cout << "Press Ctrl+C to stop the server." << std::endl;

}


void Config::_checkDuplicateListenAddresses()
{
    std::map<std::string, std::set<int> > listenAddresses;
    for (std::map<std::string, ConfigStruct>::const_iterator it = _cluster.begin(); it != _cluster.end(); ++it)
    {
        const ConfigStruct& conf = it->second;
        for (size_t i = 0; i < conf.listen.size(); ++i)
        {
            int port = conf.listen[i];
            if (listenAddresses[conf.host].count(port))
            {
                std::cerr << "Duplicate listen address found for server: " << conf.host << " on port: " << port << std::endl;
                throw std::runtime_error("Duplicate listen address found");
            }
            listenAddresses[conf.host].insert(port);
        }
    }
}
void Config::_checkBrackets(std::string all)
{
	bool openServer = false;
	bool openLocation = false;
	std::string buffer;
	std::stringstream streamBuffer;
	streamBuffer << all;
	std::stringstream serverStream;
	while(streamBuffer.good())
	{
		std::getline(streamBuffer,buffer);
		if(buffer.length() == 0)
			continue;
		serverStream << buffer << std::endl;
		if(buffer.find("server {") != std::string::npos )
		{
			if(openServer == true)
				throw Config::ServerInsideServerException();
			else
				openServer = true;
		}
		else if(buffer.find("location ") != std::string::npos)
		{
			if(buffer.find(" {") == std::string::npos)
				throw Config::WrongBlockException();
			else if(openServer == false)
				throw Config::WrongBlockException();
			else if(openLocation == true)
				throw Config::WrongBlockException();
			else
				openLocation = true;
		}
		else if(buffer.find("}") != std::string::npos)
		{
			if(openLocation == true)
				openLocation = false;
			else
			{
				openServer = false;
				this->_parseServerBlock(serverStream.str());
				serverStream.clear();
				serverStream.str(std::string());
				buffer.clear();
			}
		}
	}
	if(openLocation || openServer)
	{
		throw Config::InvalideBfracketsException();
	}
	else if(buffer.length() > 0 )
	{
		throw Config::ContentOutsideServerBlockException();
	}

}


void Config::setConfigPath(std::string configPath)
{
    this->_configPath = configPath;
}

void Config::printCluster() const
{
    for (std::map<std::string, ConfigStruct>::const_iterator it = _cluster.begin(); it != _cluster.end(); ++it) {
        const std::string &serverName = it->first;
        const ConfigStruct &conf = it->second;
        std::cout << "Server: " << serverName << std::endl;
        if (!conf.host.empty())
            std::cout << "\tHost: " << conf.host << std::endl;
        if (!conf.root.empty())
            std::cout << "\tRoot: " << conf.root << std::endl;
        if (!conf.indexPage.empty())
            std::cout << "\tIndex Page: " << conf.indexPage << std::endl;
        if (conf.autoIndex)
            std::cout << "\tAutoIndex: true" << std::endl;
        if (conf.clientMaxBodySize != 0)
            std::cout << "\tClient Max Body Size: " << conf.clientMaxBodySize << std::endl;
        if (!conf.listen.empty()) {
            std::cout << "\tListen Ports: ";
            for (size_t i = 0; i < conf.listen.size(); ++i)
                std::cout << conf.listen[i] << " ";
            std::cout << std::endl;
        }
        if (!conf.errorPage.empty()) {
            std::cout << "\tError Pages:" << std::endl;
            for (size_t i = 0; i < conf.errorPage.size(); ++i)
                std::cout << "\t\t" << conf.errorPage[i].first << " => " << conf.errorPage[i].second << std::endl;
        }
        if (!conf.location.empty()) {
            std::cout << "\tLocation blocks:" << std::endl;
            for (size_t i = 0; i < conf.location.size(); ++i) {
                std::cout << "\t\tLocation: " << conf.location[i].first << std::endl;
                const LocationStruct &loc = conf.location[i].second;
                if (!loc.root.empty())
                    std::cout << "\t\t\tRoot: " << loc.root << std::endl;

                if (!loc.indexPage.empty())
                    std::cout << "\t\t\tIndex Page: " << loc.indexPage << std::endl;

                if (loc.autoIndex)
                    std::cout << "\t\t\tAutoIndex: on" << std::endl;

                if (!loc._return.empty()) {
                    std::cout << "\t\t\tReturn: ";
                    for (size_t j = 0; j < loc._return.size(); ++j) {
                        std::cout << "(" << loc._return[j].first << ", " << loc._return[j].second << ")";
                        if (j != loc._return.size() - 1)
                            std::cout << ", ";
                    }
                    std::cout << std::endl;
                }

                if (!loc.allowedMethods.empty()) {
                    std::cout << "\t\t\tAllowed Methods: ";
                    for (std::set<std::string>::iterator mit = loc.allowedMethods.begin(); mit != loc.allowedMethods.end(); ++mit)
                        std::cout << *mit << " ";
                    std::cout << std::endl;
                }

                if (!loc.cgi_path.empty()) {
                    std::cout << "\t\t\tCGI Paths: ";
                    for (size_t j = 0; j < loc.cgi_path.size(); ++j)
                        std::cout << loc.cgi_path[j] << " ";
                    std::cout << std::endl;
                }

                if (!loc.cgi_ext.empty()) {
                    std::cout << "\t\t\tCGI Extensions: ";
                    for (size_t j = 0; j < loc.cgi_ext.size(); ++j)
                        std::cout << loc.cgi_ext[j] << " ";
                    std::cout << std::endl;
                }

                if (loc.upload_enabled)
                    std::cout << "\t\t\tUpload_enabled: on" << std::endl;

                if (!loc.upload_path.empty())
                    std::cout << "\t\t\tUpload_path: " << loc.upload_path << std::endl;
            }
        }
        std::cout << std::endl;
    }
}

int Config::getAutoindex()
{
    return _cluster.begin()->second.autoIndex;
}


void Config::_checkRedirectionLoops()
{
	for (std::map<std::string, ConfigStruct>::iterator it = _cluster.begin(); it != _cluster.end(); ++it)
	{
		ConfigStruct &conf = it->second;
		for (size_t i = 0; i < conf.location.size(); ++i)
		{
			LocationStruct &loc = conf.location[i].second;
			for (size_t j = 0; j < loc._return.size(); ++j)
			{
				const std::string &target = loc._return[j].second;
				if (target.find("http://") == 0 || target.find("https://") == 0)
					continue;
				bool found = false;
				for (std::map<std::string, ConfigStruct>::iterator it2 = _cluster.begin(); it2 != _cluster.end(); ++it2) {
					ConfigStruct &conf2 = it2->second;

					for (size_t k = 0; k < conf2.location.size(); ++k) {
						const std::string &locPath = conf2.location[k].first;
						if (target == locPath) {
							found = true;
							break;
						}
					}
					if (found) break;
				}
				if (!found)
					throw std::runtime_error("Redirection loop detected for target: " + target);
			}
		}
	}

}
const char* Config::FileOpenException::what(void) const throw()
{
	return ("Failed to read from .conf file, check file existence and readrights");
}
const char* Config::ServerInsideServerException::what(void) const throw()
{
	return ("Wrong Syntax in .conf file, server-block inside server-block found");
}
const char* Config::WrongBlockException::what(void) const throw()
{
	return ("Wrong location-block found inside .conf file");
}
const char* Config::InvalideBfracketsException::what(void) const throw()
{
	return("Invalid brackets in .conf file");
}
const char* Config::ContentOutsideServerBlockException::what(void) const throw()
{
	return("Content is forbidden outside server-block");
}
