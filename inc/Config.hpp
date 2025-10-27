#ifndef CONFIG_HPP
#define CONFIG_HPP
#pragma once


#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <stdexcept>
// #include "../inc/webserv.hpp"
#include "singleserver.hpp"
#include "Struct.hpp"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"


class Config
{
    private:
        std::ifstream	_configFile;
		std::string		_configPath; 
        Config(const Config&);
        void _parseServerBlock(std::string serverBlock);
        void _createConfigStruct(std::string server); 
    public:
        std::map<std::string, ConfigStruct> _cluster;
        Config();
        ~Config();
        void StartToSet(std::string configPath);
        void _checkDuplicateListenAddresses();
        void _checkBrackets(std::string buffer);
        void setConfigPath(std::string configPath);
        void printCluster() const;
        int getAutoindex();
        void _checkRedirectionLoops();
        class FileOpenException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};
        class ServerInsideServerException : public std::exception
        {
            public:
                virtual const char* what () const throw();
        };
        class  WrongBlockException : public std::exception
        {
            public:
                virtual const char* what() const throw();
        };
        class InvalideBfracketsException :  public std::exception
        {
            public:
                virtual const char* what() const throw();
        };
        class ContentOutsideServerBlockException: public std::exception
        {
            public:
                virtual const char* what() const throw();
        };


};

std::string parseArgv(int argc, char **argv);


#endif
