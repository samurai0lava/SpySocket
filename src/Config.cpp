#include "../include/Config.hpp"


std::string parseArgv(int argc, char** argv)
{
	std::string defaultConfPath = "file.conf";
	if (argc == 1)
	{
		return (defaultConfPath);
	}
	else if (argc > 2)
	{
		std::cout << "Please use webserv with config file only as follows:" << std::endl << "./webserv <config_filename.conf>" << std::endl;
		exit(1);
	}
	std::string sArgv = argv[1];
	std::string ending = ".conf";
	if ((argv[1] + sArgv.find_last_of(".")) != ending)
	{
		std::cout << "Please use webserv with config file only as follows:" << std::endl << "./webserv <config_filename.conf>" << std::endl;
		exit(1);
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
		{
			continue ;
		}
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
			server.append(buffer);//add length of buffer?
			server.append("\n");
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
			{
				throw Config::ServerInsideServerException();
			}
			else
			{
				openServer = true;
			}
		}
		else if(buffer.find("location ") != std::string::npos)
		{
			if(buffer.find(" {") == std::string::npos)
			{
				throw Config::WrongBlockException();
			}
			else if(openServer == false)
			{
				
				throw Config::WrongBlockException();
			}
			else if(openLocation == true)
			{
				throw Config::WrongBlockException();
			}
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

void Config::printCluster() const {
	for (std::map<std::string, ConfigStruct>::const_iterator it = _cluster.begin(); it != _cluster.end(); ++it) {
		const std::string &serverName = it->first;
		const ConfigStruct &conf = it->second;

		std::cout << "Server: " << serverName << std::endl;
		std::cout << "\tHost: " << conf.host << std::endl;
		std::cout << "\tRoot: " << conf.root << std::endl;
		std::cout << "\tIndex Page: " << conf.indexPage << std::endl;
		std::cout << "\tAutoIndex: " << (conf.autoIndex ? "true" : "false") << std::endl;
		std::cout << "\tClient Max Body Size: " << conf.clientMaxBodySize << std::endl;

		std::cout << "\tListen Ports: ";
		for (size_t i = 0; i < conf.listen.size(); ++i)
			std::cout << conf.listen[i] << " ";
		std::cout << std::endl;

		std::cout << "\tError Pages:" << std::endl;
		for (size_t i = 0; i < conf.errorPage.size(); ++i)
			std::cout << "\t\t" << conf.errorPage[i].first << " => " << conf.errorPage[i].second << std::endl;

		std::cout << "\tLocation blocks:" << std::endl;
		for (size_t i = 0; i < conf.location.size(); ++i) {
			std::cout << "\t\tLocation: " << conf.location[i].first << std::endl;
			const LocationStruct &loc = conf.location[i].second;
			std::cout << "\t\t\tRoot: " << loc.root << std::endl;
			std::cout << "\t\t\tIndex Page: " << loc.indexPage << std::endl;
			std::cout << "\t\t\tAutoIndex: " << (loc.autoIndex ? "true" : "false") << std::endl;
			std::cout << "\t\t\tReturn: ";
			for (size_t i = 0; i < loc._return.size(); ++i)
			{
				std::cout << "(" << loc._return[i].first << ", " << loc._return[i].second << ")";
				if (i != loc._return.size() - 1)
					std::cout << ", ";
			}
			std::cout << std::endl;
			std::cout << "\t\t\tAllowed Methods: ";
			for (std::set<std::string>::iterator mit = loc.allowedMethods.begin(); mit != loc.allowedMethods.end(); ++mit)
				std::cout << *mit << " ";
			std::cout << std::endl;

			std::cout << "\t\t\tCGI Paths: ";
			for (size_t j = 0; j < loc.cgi_path.size(); ++j)
				std::cout << loc.cgi_path[j] << " ";
			std::cout << std::endl;

			std::cout << "\t\t\tCGI Extensions: ";
			for (size_t j = 0; j < loc.cgi_ext.size(); ++j)
				std::cout << loc.cgi_ext[j] << " ";
			std::cout << std::endl;
			std::cout << "\t\t\tUpload_enabled: " << (loc.upload_enabled ? "on" : "off") << std::endl;
			std::cout << "\t\t\tUpload_path : " << loc.upload_path << std::endl;
		}
		std::cout << std::endl;
	}
}


int Config::getAutoindex()
{
    return _cluster.begin()->second.autoIndex;
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