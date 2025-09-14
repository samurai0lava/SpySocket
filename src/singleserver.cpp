#include "../include/singleserver.hpp"

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
         throw SingleServerConfig::NoListenException();
	}
    std::string key = keyValue.substr(0, keyValue.find_first_of("\n\r\t\f\v "));
	std::string value = "";
	int nKey = 0;
    for (;nKey < 11; ++nKey)
	{
		if (configVariables[nKey] == key){
			break ;
        }
	}
    switch (nKey)
	{
        case (listen_):
        {
            value = keyValue.substr(keyValue.find_first_of("\n\r\t\f\v ") + 1);
            if (value.find_first_not_of(DECIMAL) != std::string::npos)
		    {
                std::cout << keyValue << std::endl;
                throw SingleServerConfig::NoListenException();
		    }
            std::stringstream ss(value);
            unsigned short port;
            ss >> port;
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
            if(this->_conf->root.length() != 0)//check Prevents multiple root directives
            {
                throw SingleServerConfig::DublicateRootException();
            }
		    value = keyValue.substr(keyValue.find_first_of("\n\r\t\f\v ") + 1);
            if (value[0] != '/' || value[value.length() - 1] != '/')
            {
                throw SingleServerConfig::InvalidPathException();
            }
            this->_conf->root = value;
            break;

        }
        case(server_name):
        {
            if (this->_conf->serverName.length() != 0)
		    {
			    throw SingleServerConfig::DuplicateServerNameException();
		    }
            value = keyValue.substr(keyValue.find_first_of("\n\r\t\f\v ") + 1);
            this->_conf->serverName = value;
            break;
        }
        case(index_page):
        {
            if (this->_conf->indexPage.length() != 0)
		    {
                throw std::runtime_error("Duplicate 'index_page' directive.");
		    }
            value = keyValue.substr(keyValue.find_first_of("\n\r\t\f\v ") + 1);
            this->_conf->indexPage = value;
            break;
        }
        case(autoindex):
        {
            // if (this->_conf->autoIndex == true)
            //     throw std::runtime_error("Duplicate autoindex directive found.");
            value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
		    value.erase(0, value.find_first_not_of(WHITESPACE));
            value.erase(value.find_last_not_of(WHITESPACE) + 1);
            if (value != "true" && value != "false")
                throw std::runtime_error("Invalid value for 'autoindex': expected 'true' or 'false'.");
            if (value == "true") {
                this->_conf->autoIndex = true;
            } else {
                this->_conf->autoIndex = false;
            }
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
            if (cbbsSet == true)
		    {
                throw std::runtime_error("Duplicate Client Max Body Size.");
            }
            else
            {
                cbbsSet = true;}
            size_t out = 0;
		    value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
            if(value.find_first_not_of(DECIMAL) != std::string::npos )
                throw std::runtime_error("this does not fit the required argument type.");
            std::istringstream(value.c_str()) >> out;
		    this->_conf->clientMaxBodySize = out;

            break;
        }
        default:
        {
            std::cout<<"---> "<<keyValue<<" <---"<<std::endl;
            throw std::runtime_error("Invalid key ");
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
     std::istringstream iss(line);
    std::string directive, statusCode, path, extra;
    iss >> directive >> statusCode >> path;
    iss >> extra;
    if (statusCode.empty() || path.empty() || !extra.empty())
        throw std::runtime_error("Error: 'error_page' must have exactly 2 arguments: status_code and file_path.");

    for (size_t i = 0; i < this->_conf->errorPage.size(); ++i)
    {
        if (this->_conf->errorPage[i].first == statusCode)
            throw std::runtime_error("Duplicate error_page entry found for status code: " + statusCode);
    }

    this->_conf->errorPage.push_back(std::make_pair(statusCode, path));

}
std::string locationVariables1[] =
    {
        "root",
        "autoindex",
        "method",
        "index_page",
        "_return",
        "cgi_path",
        "cgi_ext",
        "upload_enabled",
        "upload_path"
    };
LocationStruct SingleServerConfig::_fillLocationStruct(std::string block)
{
    LocationStruct location_tmp;
    location_tmp.autoIndex = false;
    location_tmp.upload_enabled = true; //was false ta nrj3ha
    location_tmp.indexPage = "";
    location_tmp.root = "";
    std::stringstream bstream;
    bstream << block;
    std::string keyValue;
    std::string key;
    bool foundRoot = false;
	bool foundMethod = false;
	bool foundIndex = false;
	bool foundAutoIndex = false;
    bool foundUpload_enabled = false;
    std::string value;

    while(bstream.good())
    {

        std::getline(bstream, keyValue);
		key = keyValue.substr(0, keyValue.find_first_of(WHITESPACE));
        if(key =="method" &&  keyValue.find_first_of(WHITESPACE) == std::string::npos && foundMethod == false)
        {
            foundMethod = true;
            continue;
        }
		size_t foundKey = location_root;
        for (; foundKey < upload_path  + 1; ++foundKey)
		{
			if (locationVariables1[foundKey] == key){
				break ;}
		}
        switch (foundKey)
		{
            case (location_root):
            {
                if (foundRoot == true)
				    throw std::runtime_error ("Duplicate location root.");
			    value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
                location_tmp.root = value;
			    foundRoot = true;
                break;

            }
            case (method):
		    { 
                if (foundMethod == true)
                    throw std::runtime_error("Duplicate Method Exception .");
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
                if (foundIndex == true)
                    throw std::runtime_error("Duplicate Location Index");
			    value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
                location_tmp.indexPage = value;
			    foundIndex = true;
			    break ;
            }
            case(location_auto_index):
            {
                if (foundAutoIndex == true)
                    throw std::runtime_error("Duplicate Location auto index");
			    value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
                location_tmp.autoIndex = (value.compare("true") == 0);
			    foundAutoIndex = true;
                break;
            }
            case(_return):
            {
                std::istringstream iss(keyValue);
                std::string directive, statusCode, redirectUrl;
                iss >> directive >> statusCode >> redirectUrl;
                std::string extra;

                iss >> extra;
                // if (statusCode.empty() || redirectUrl.empty() || !extra.empty())
                //     throw std::runtime_error("Error: _return must have exactly 2 arguments: status_code and URL.");

                if (!location_tmp._return.empty())
                    throw std::runtime_error("Error: Multiple return directives are not allowed in one location block.");

                location_tmp._return.push_back(std::make_pair(statusCode, redirectUrl));
                break;
            }
            case(cgi_path):
            {
               std::string value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
                std::stringstream ss(value);
                std::string path;
                while (ss >> path)
                    location_tmp.cgi_path.push_back(path);
                break;
            }
            case(cgi_ext):
            {
                std::string value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
                std::stringstream ss(value);
                std::string path;
                while (ss >> path)
                    location_tmp.cgi_ext.push_back(path);
                break;
            }
            case(upload_enabled):
            {
                // std::cout<<"1"<<std::endl;
                if (foundUpload_enabled == true)
                    throw std::runtime_error("Duplicate Location Upload enabled");
			    value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
                location_tmp.upload_enabled = (value.compare("on") == 0);
			    foundUpload_enabled = true;
                break;
            }
            case(upload_path):
            {
                // std::cout<<"KeyValue : "<<keyValue.c_str()<<std::endl;
                value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
                location_tmp.upload_path = value;                
                break;

            }
            // default: /// trj3 liha waaaaa
            // {
            //     std::cout<<"key is : "<<keyValue<<"hello"<<std::endl;
            //     throw std::runtime_error("Invalid key for location ");
            //     break ;
            // }
        }

    }
    return LocationStruct(location_tmp);
}

SingleServerConfig::SingleServerConfig(std::string server, ConfigStruct *conf) : _conf(conf)
{
	this->cbbsSet = false;
    this->_setVariables(server);
 
}
const char* SingleServerConfig::NoListenException::what(void) const throw()
{
	return ("Error: 'listen' directive is required and must be properly formatted.");
}
const char * SingleServerConfig::DublicateRootException:: what(void) const throw()
{
    // return ("")
	return ("only one location-block with same key allowed");

}
const char* SingleServerConfig::InvalidPathException::what(void) const throw()
{
	return ("this path is invalid, has to be like : \"/path/ and no use of '.'\"");
}

const char* SingleServerConfig::DuplicateServerNameException::what(void) const throw()
{
	return ("duplicate Server Name ."); // lalal
}