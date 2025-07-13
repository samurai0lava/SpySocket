#include "Config.hpp"


void Config::_parseServerBlock(std::string serverBlock)
{
	std::string server;
	std::string buffer;
	std::stringstream serverStream;
	serverStream << serverBlock;
	bool serverFound = false;
	while(serverStream.good())
	{
		buffer.clear();
		std::getline(serverStream, buffer);
		// std::cout<<"line : "<<buffer<<std::endl;
		if (buffer.length() == 0)
			continue ;
		size_t start = buffer.find_first_not_of("\n\r\t\f\v ");
		// std::cout<<"start : "<<start<<std::endl;
		if (start == std::string::npos)
		{
			continue ;
		}
		size_t end = buffer.find_first_of(";");
		// std::cout<<"the end : "<<end<<std::endl;
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
			server.append(buffer);
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
	tmp.autoIndex = false;
	if (server.find("server_name") == std::string::npos)
	{
		std::cout<<"Throwing exception..."<<std::endl;
		return ;
	}
	std::string serverName = server.substr(server.find("server_name"));
	serverName = serverName.substr(serverName.find_first_of(" ") + 1);
	serverName = serverName.substr(0, serverName.find_first_of("\n"));
	ConfigStruct confStruct = tmp;
	SingleServerConfig temp(server, &confStruct);

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
	int i = 0;
	std::stringstream serverStream;
	while(streamBuffer.good())
	{
		std::getline(streamBuffer,buffer);
		if(buffer.length() == 0)
			continue;
		serverStream << buffer << std::endl;
		// std::cout<<"hello : "<<serverStream.str()<<std::endl;
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