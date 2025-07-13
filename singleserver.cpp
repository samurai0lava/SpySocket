#include "singleserver.hpp"

void SingleServerConfig::_parseKeyValue(std::string keyValue)
{
    std::string configVariables[]=
    {
        "listen",
        "root",
        "server_name",
        "index_page",
        "location",
        "error_page",
        "autoindex",
        "host",
        "client_max_body_size",
    };
    if (keyValue.find_first_of("\n\r\t\f\v ") == std::string::npos)
	{
		std::cout << keyValue <<  std::endl;
        return ;
	}
    std::string key = keyValue.substr(0, keyValue.find_first_of("\n\r\t\f\v "));
	std::string value = "";
	int nKey = 0;
    for (;nKey < 11; ++nKey)
	{
		if (configVariables[nKey] == key){
			break ;}
	}
    switch (nKey)
	{
        case (listen_):
        {
		    value = keyValue.substr(keyValue.find_first_of("\n\r\t\f\v ") + 1);
            std::stringstream ss(value);
            unsigned short port;
            ss >> port;

            // std::cout << "PORT : " << port << " VALUE : " << value << std::endl;

            bool exists = false;
            for (size_t i = 0; i < this->_conf->listen.size(); ++i)
            {
                if (this->_conf->listen[i] == port)
                {
                    exists = true;
                    break;
                }
            }
            if (!exists)
            {
                this->_conf->listen.push_back(port);
            }
            break;

            
        }
        case(host):
        {
            value = keyValue.substr(keyValue.find_first_of("\n\r\t\f\v ") + 1);
            this->_conf->host = value;
            break;

        }
        case(root):
        {
		    value = keyValue.substr(keyValue.find_first_of("\n\r\t\f\v ") + 1);
            this->_conf->root = value;
            break;

        }
        case(server_name):
        {
            value = keyValue.substr(keyValue.find_first_of("\n\r\t\f\v ") + 1);
            this->_conf->serverName = value;
            break;
        }
        case(index_page):
        {
            value = keyValue.substr(keyValue.find_first_of("\n\r\t\f\v ") + 1);
            this->_conf->indexPage = value;
            break;
        }
        case(autoindex):
        {
		    value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
            this->_conf->autoIndex = (value.compare("true") == 0);
		    break ;

        }
        case(location):
        {
            this->_handleLocation(keyValue);
            break;
        }
        case (error_page):
        {
            this->_handleErrorPage(keyValue);
            break ;
        }
        case(client_max_body_size):
        {
            size_t out = 0;
		    value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
            std::istringstream(value.c_str()) >> out;
		    this->_conf->clientMaxBodySize = out;

            break;
        }
    }

    
}

void SingleServerConfig::_setVariables(std::string config)
{
  
	std::stringstream configStream(config);

	std::string buffer = "";
	while (buffer.find("{") == std::string::npos && configStream.good())
	{
		buffer.clear();
		std::getline(configStream, buffer);
	}
	buffer.clear();
	while (std::getline(configStream, buffer) && configStream.good() && buffer != "}")
	{
		if (buffer.find("location") != std::string::npos)
		{
			std::stringstream locationBlock;
			locationBlock << buffer << std::endl;

			while (std::getline(configStream, buffer) && configStream.good())
			{
				locationBlock << buffer << std::endl;
				if (buffer == "}")
					break ;
			}
			buffer.clear();
			buffer = locationBlock.str();
		}
		this->_parseKeyValue(buffer);
		buffer.clear();
	}
}

void SingleServerConfig::_handleLocation(std::string block)
{
    std::stringstream bufferStream;
	bufferStream << block;
	std::string line;
	std::getline(bufferStream, line);
	std::string key = line.substr(line.find_first_of("\n\r\t\f\v ") + 1);
    key = key.substr(0, key.find_first_of("\n\r\t\f\v "));
    bool openBrackets = true;
	std::stringstream blockStream;
    while (bufferStream.good() && openBrackets == true)
    {
        std::getline(bufferStream, line);
        if (line == "}")
            openBrackets = false;
        else if (openBrackets == false)
        {
            exit(0);
        }
        else
            blockStream << line << std::endl;
    }
    LocationStruct tmp = this->_fillLocationStruct(blockStream.str());
    this->_conf->location.push_back(std::make_pair(key, tmp));

}
void SingleServerConfig::_handleErrorPage(std::string line)
{
    std::string key = line.substr(line.find_first_of(WHITESPACE) + 1);
	std::string value = key.substr(key.find_first_of(WHITESPACE) + 1);
	key = key.substr(0, key.find_first_of(WHITESPACE));
	this->_conf->errorPage.push_back(std::make_pair<std::string, std::string>(key, value));

}
std::string locationVariables1[] =
    {
        "root",
        "autoindex",
        "method",
        "index_page",
        "_return",
        "cgi_path",
        "cgi_ext"
    };
LocationStruct SingleServerConfig::_fillLocationStruct(std::string block)
{
    LocationStruct location_tmp;
    location_tmp.autoIndex = false;
    location_tmp.indexPage = "";
    location_tmp.root = "";
    location_tmp._return = "";
    std::stringstream bstream;
    bstream << block;
    std::string keyValue;
    std::string key;
    bool foundRoot = false;
	bool foundMethod = false;
	bool foundIndex = false;
	bool foundAutoIndex = false;
    std::string value;

    while(bstream.good())
    {
        // std::cout << "line: >" << keyValue << "<" << std::endl;
        std::getline(bstream, keyValue);
		key = keyValue.substr(0, keyValue.find_first_of(WHITESPACE));
        if(key =="method" &&  keyValue.find_first_of(WHITESPACE) == std::string::npos && foundMethod == false)
        {
            std:
            foundMethod = true;
            continue;
        }
		size_t foundKey = location_root;
        for (; foundKey < cgi_ext + 1; ++foundKey)
		{
			if (locationVariables1[foundKey] == key){
				break ;}
		}
        // std::cout<<"Switch for :"<<key<<"<-----Found key:"<<foundKey<<std::endl;
        switch (foundKey)
		{
            case (location_root):
            {
			    value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
                location_tmp.root = value;
			    foundRoot = true;
                break;

            }
            case (method):
		    { 
			    value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
                for (; value.length() > 0;)
			    {
                    std::string tempValue = value.substr(0, value.find_first_of(WHITESPACE));
                    if((tempValue == "GET" || tempValue == "POST" || tempValue == "DELETE") && location_tmp.allowedMethods.count(tempValue) == 0)
					    location_tmp.allowedMethods.insert(tempValue);
                    if (value.find_first_of(WHITESPACE) != std::string::npos)
					    value = value.substr(value.find_first_of(WHITESPACE) + 1);
				    else
					    value = "";
                }
			    foundMethod = true;
                break;
            }
            case(location_index):
            {
			    value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
                location_tmp.indexPage = value;
			    foundIndex = true;
			    break ;
            }
            case(location_auto_index):
            {
			    value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
                location_tmp.autoIndex = (value.compare("true") == 0);
			    foundAutoIndex = true;
                break;
            }
            case(_return):
            {
                value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
                location_tmp._return = value;
                break;
            }
            case(cgi_path):
            {
               std::string value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
                std::stringstream ss(value);
                std::string path;
                // std::cout<<"lol78\n"<<std::endl;
                while (ss >> path)
                    location_tmp.cgi_path.push_back(path);
                break;
            }
            case(cgi_ext):
            {
                std::string value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
                std::stringstream ss(value);
                std::string path;
                // std::cout<<"lol78\n"<<std::endl;
                while (ss >> path)
                    location_tmp.cgi_ext.push_back(path);
                break;
            }
        }

    }
    return LocationStruct(location_tmp);
}

SingleServerConfig::SingleServerConfig(std::string server, ConfigStruct *conf) : _conf(conf)
{
    // std::cout<<"lol\n";
    this->_setVariables(server);
    printConfigStruct(*_conf);
 
}