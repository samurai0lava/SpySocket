#include "../include/singleserver.hpp"

void SingleServerConfig::_parseKeyValue(std::string keyValue)
{
    std::string configVariables[]=
    {
        "listen",
        "root",
        "server_name",
        "location",
        "error_page",
        "host",
        "client_max_body_size",
    };
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
            if (value.find_first_not_of(DECIMAL) != std::string::npos){
                std::cout<<"hello2"<<std::endl;
                throw SingleServerConfig::NoListenException();}
            int iport = atoi(value.c_str());
            if (iport <= 0 || iport > 65535) 
                throw std::runtime_error("Invalid listen port: must be between 1 and 65535");
            std::stringstream ss(value);
            unsigned short port;
            ss >> port;
            bool exists = false;
            for (size_t i = 0; i < this->_conf->listen.size(); ++i)
            {
                if (this->_conf->listen[i] == port)
                    throw std::runtime_error("Duplicate listen port: " + value);
            }
            if (!exists)
                this->_conf->listen.push_back(port);
            break;

            
        }
        case(host):
        {
            value = keyValue.substr(keyValue.find_first_of("\n\r\t\f\v ") + 1);
             if (!_isValidHost(value))
                throw std::runtime_error("Invalid host format: " + value);
            this->_conf->host = value;
            break;

        }
        case(root):
        {
            if(this->_conf->root.length() != 0)
                throw SingleServerConfig::DublicateRootException();
		    value = keyValue.substr(keyValue.find_first_of("\n\r\t\f\v ") + 1);
            // tanswal chey hed ....
            // if (value[0] != '/' || value[value.length() - 1] != '/')
            // {
            //     throw SingleServerConfig::InvalidPathException();
            // }
            this->_conf->root = value;
            break;

        }
        case(server_name):
        {
            if (this->_conf->serverName.length() != 0)
			    throw SingleServerConfig::DuplicateServerNameException();
            value = keyValue.substr(keyValue.find_first_of("\n\r\t\f\v ") + 1);
            this->_conf->serverName = value;
            break;
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
                throw std::runtime_error("Duplicate Client Max Body Size.");
            else
                cbbsSet = true;
             value = keyValue.substr(keyValue.find_first_of("\n\r\t\f\v ") + 1);
            size_t start = value.find_first_not_of(" \t\n\r\f\v");
            if (start != std::string::npos) {
                size_t end = value.find_last_not_of(" \t\n\r\f\v");
                value = value.substr(start, end - start + 1);
            }
            size_t bodySize = _parseBodySize(value);
            this->_conf->clientMaxBodySize = bodySize;
            // size_t out = 0;
		    // value = keyValue.substr(keyValue.find_first_of(WHITESPACE) + 1);
            // if(value.find_first_not_of(DECIMAL) != std::string::npos )
            //     throw std::runtime_error("this does not fit the required argument type.");
            // std::istringstream(value.c_str()) >> out;
		    // this->_conf->clientMaxBodySize = out;

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
        // std::cout<<"Buffer : "<<buffer.c_str()<<std::endl;
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
    if(!_isValidLocationPath(key))
       throw std::runtime_error("Invalid location path: " + key);
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
    
    int code = std::atoi(statusCode.c_str());
    if (code < 400 || code > 599)
        throw std::runtime_error("Invalid error status code: " + statusCode);
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
    LocationStruct loc;
    _initializeLocationStruct(loc);
    std::istringstream iss(block);
    std::string line;
    
    while (std::getline(iss, line))
    {
      
        size_t start = line.find_first_not_of(" \t\n\r\f\v");
        if (start == std::string::npos)
            continue;
        size_t end = line.find_last_not_of(" \t\n\r\f\v");
        line = line.substr(start, end - start + 1);
        
        if (line.empty() || line[0] == '#')
            continue; 
        if (!line.empty() && line[line.length() - 1] == ';')
            line = line.substr(0, line.length() - 1);
        _parseLocationDirective(loc, line);
    }
    
    return loc;
}


void SingleServerConfig::_parseLocationDirective(LocationStruct& loc, const std::string& directive)
{
    std::string locationDirectives[] = {
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
    
    if (directive.find_first_of(" \t") == std::string::npos)
    {
        throw std::runtime_error("Invalid location directive format: " + directive);
    }
    std::string key = directive.substr(0, directive.find_first_of(" \t"));
    std::string value = directive.substr(directive.find_first_of(" \t") + 1);
    size_t start = value.find_first_not_of(" \t\n\r\f\v");
    if (start != std::string::npos) {
        size_t end = value.find_last_not_of(" \t\n\r\f\v");
        value = value.substr(start, end - start + 1);
    }
    int nKey = 0;
    for (; nKey < 9; ++nKey)
    {
        if (locationDirectives[nKey] == key)
            break;
    }
    
    switch (nKey)
    {
        case 0: // root
            _parseLocationRoot(loc, value);
            break;
        case 1: // autoindex
            _parseLocationAutoindex(loc, value);
            break;
        case 2: // method
            _parseLocationMethod(loc, value);
            break;
        case 3: // index_page
            _parseLocationIndexPage(loc, value);
            break;
        case 4: // _return
            _parseLocationReturn(loc, value);
            break;
        case 5: // cgi_path
            _parseLocationCgiPath(loc, value);
            break;
        case 6: // cgi_ext
            _parseLocationCgiExt(loc, value);
            break;
        case 7: // upload_enabled
            _parseLocationUploadEnabled(loc, value);
            break;
        case 8: // upload_path
            _parseLocationUploadPath(loc, value);
            break;
        default:
            throw std::runtime_error("Unknown location directive: " + key);
    }

}

void SingleServerConfig::_parseLocationRoot(LocationStruct& loc, const std::string& value)
{
    if (!loc.root.empty())
        throw std::runtime_error("Duplicate root directive in location");
    
    if (value.empty())
        throw std::runtime_error("Empty root value in location");
    
    loc.root = value;
}

void SingleServerConfig::_parseLocationAutoindex(LocationStruct& loc, const std::string& value)
{
    if (value == "on")
        loc.autoIndex = true;
    else if (value == "off")
        loc.autoIndex = false;
    else
        throw std::runtime_error("Invalid autoindex value: " + value + " (use on/off)");
}

void SingleServerConfig::_parseLocationMethod(LocationStruct& loc, const std::string& value)
{
    std::istringstream iss(value);
    std::string method;
    
    while (iss >> method)
    {
        if (method != "GET" && method != "POST" && method != "DELETE" )
            throw std::runtime_error("Invalid HTTP method: " + method);
        loc.allowedMethods.insert(method);
    }
    if (loc.allowedMethods.empty())
        throw std::runtime_error("No methods specified in method directive");
}

void SingleServerConfig::_parseLocationIndexPage(LocationStruct& loc, const std::string& value)
{
    if (!loc.indexPage.empty())
        throw std::runtime_error("Duplicate index_page directive in location");
    
    if (value.empty())
        throw std::runtime_error("Empty index_page value in location");
    
    loc.indexPage = value;
}

void SingleServerConfig::_parseLocationReturn(LocationStruct& loc, const std::string& value)
{
    std::istringstream iss(value);
    std::string code, url;
    
    if (!(iss >> code >> url))
        throw std::runtime_error("Invalid _return format: expected 'code url'");
    int statusCode = std::atoi(code.c_str());
    if (statusCode < 300 || statusCode > 399)
        throw std::runtime_error("Invalid redirect status code: " + code);
    loc._return.push_back(std::make_pair(code, url));
}
void SingleServerConfig::_parseLocationCgiPath(LocationStruct& loc, const std::string& value)
{
    std::istringstream iss(value);
    std::string path;
    
    while (iss >> path)
    {
        loc.cgi_path.push_back(path);
    }
}

void SingleServerConfig::_parseLocationCgiExt(LocationStruct& loc, const std::string& value)
{
    std::istringstream iss(value);
    std::string ext;
    while (iss >> ext)
    {
        if (ext[0] != '.')
            ext = "." + ext; 
        loc.cgi_ext.push_back(ext);
    }
}

void SingleServerConfig::_parseLocationUploadEnabled(LocationStruct& loc, const std::string& value)
{
    if (value == "on" || value == "true")
        loc.upload_enabled = true;
    else if (value == "off" || value == "false")
        loc.upload_enabled = false;
    else
        throw std::runtime_error("Invalid upload_enabled value: " + value + " (use on/off)");
}

void SingleServerConfig::_parseLocationUploadPath(LocationStruct& loc, const std::string& value)
{
    if (!loc.upload_path.empty())
        throw std::runtime_error("Duplicate upload_path directive in location");
    
    loc.upload_path = value;
}
bool SingleServerConfig::_isValidHost(const std::string& host) const
{
    if (host.empty() || host.length() > 253)
        return false;
        
    std::string hostToCheck = host;
    if (host[0] == '*')
    {
        if (host.length() < 2 || host[1] != '.')
            return false;
        hostToCheck = host.substr(2);
    }
    bool looksLikeIP = true;
    for (size_t i = 0; i < hostToCheck.length(); ++i)
    {
        if (!std::isdigit(hostToCheck[i]) && hostToCheck[i] != '.')
        {
            looksLikeIP = false;
            break;
        }
    }
    if (looksLikeIP)
    {
        return _isValidIPv4(hostToCheck);
    }
    return _isValidDomain(hostToCheck);
}
bool SingleServerConfig::_isValidIPv4(const std::string& ip) const
{
    std::istringstream iss(ip);
    std::string octet;
    int count = 0;
    
    while (std::getline(iss, octet, '.'))
    {
        count++;
        if (count > 4)
            return false;
        
        if (octet.empty() || octet.length() > 3)
            return false;
        
        for (size_t i = 0; i < octet.length(); ++i)
        {
            if (!std::isdigit(octet[i]))
                return false;
        }
        if (octet.length() > 1 && octet[0] == '0')
            return false;
        
        int num = std::atoi(octet.c_str());
        if (num < 0 || num > 255){
            return false;}
    }
    bool result = (count == 4);
    return result;
}
bool SingleServerConfig::_isValidDomain(const std::string& domain) const
{
      if (domain.empty() || domain.length() > 253)
        return false;
    if (domain[0] == '.' || domain[0] == '-' || 
        domain[domain.length() - 1] == '.' || domain[domain.length() - 1] == '-')
        return false;
    
    std::istringstream iss(domain);
    std::string label;
    
    while (std::getline(iss, label, '.'))
    {
        if (label.empty() || label.length() > 63)
            return false;
        if (label[0] == '-' || label[label.length() - 1] == '-')
            return false;
        for (size_t i = 0; i < label.length(); ++i)
        {
            char c = label[i];
            if (!std::isalnum(c) && c != '-')
                return false;
        }
    }
    
    return true;
}


size_t SingleServerConfig::_parseBodySize(const std::string& sizeStr) const
{
    if (sizeStr.empty())
        throw std::runtime_error("Empty client_max_body_size value");
    
    std::string numberPart;
    std::string suffix;
    size_t i = 0;
    while (i < sizeStr.length() && std::isdigit(sizeStr[i])){
        numberPart += sizeStr[i];
        i++;}
    while (i < sizeStr.length())
    {
        suffix += std::toupper(sizeStr[i]);
        i++;
    }
    if (numberPart.empty())
        throw std::runtime_error("No numeric value in client_max_body_size: " + sizeStr);
    if (!suffix.empty() &&  suffix != "KB" &&  suffix != "MB"  && suffix != "GB")
        throw std::runtime_error("Invalid size suffix in client_max_body_size: " + sizeStr);
    
    long long number = std::atoll(numberPart.c_str());
    if (number < 0)
        throw std::runtime_error("Negative value not allowed in client_max_body_size");
    size_t result = number;
 
    if ( suffix == "KB")
        result *= 1024;
    else if ( suffix == "MB")
        result *= 1024 * 1024;
    else if ( suffix == "GB")
        result *= 1024 * 1024 * 1024;
    if (result > 2147483648UL) 
        throw std::runtime_error("client_max_body_size too large (max 2GB)");
    return result;
}


bool SingleServerConfig::_isValidLocationPath(const std::string& path) const
{
    if (path.empty())
    return false;
    
    if (path[0] != '/')
        return false;
    
    if (path.find('\0') != std::string::npos)
        return false;
    if (path.find("../") != std::string::npos || path.find("/..") != std::string::npos)
        return false;
    if (path.find("//") != std::string::npos)
        return false;
    return true;

}

void SingleServerConfig::_initializeLocationStruct(LocationStruct& loc) const
{
    loc.autoIndex = false;
    loc.root = "";
    loc.indexPage = "";
    loc.upload_enabled = false;
    loc.upload_path = "";
    loc.allowedMethods.clear();
    loc._return.clear();
    loc.cgi_path.clear();
    loc.cgi_ext.clear();
    loc.url_encoded.clear();
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