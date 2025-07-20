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
#include "singleserver.hpp"
#include "Struct.hpp"


class Config
{
    private:
        std::ifstream	_configFile;
		std::string		_configPath; 
		ConfigStruct _conf; 
        Config(const Config&);
        void _parseServerBlock(std::string serverBlock);
        void _createConfigStruct(std::string server); 
    public:
        std::map<std::string, ConfigStruct> _cluster;
        Config();
        ~Config();
        void StartToSet(std::string configPath);
        void _checkBrackets(std::string buffer);
        void setConfigPath(std::string configPath);
        void printCluster() const;
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

static std::string parseArgv(int argc, char **argv);


#endif
