
#ifndef SINGLESERVER_HPP
#define SINGLESERVER_HPP
#pragma once
#include <string>
#include <iostream>
#include <utility>
#include <sstream>
#include <stdbool.h>
#include <map>
#include <vector>
#include <climits>
#include <cstdlib>
#include "Config.hpp"
#include "Struct.hpp"
 
class SingleServerConfig
{
	private:
		ConfigStruct *_conf;
		bool cbbsSet;
		void _parseKeyValue(std::string);
		void _setVariables(std::string config);
		void _handleLocation(std::string line);
		void _handleErrorPage(std::string line);
		LocationStruct _fillLocationStruct(std::string block);
		void _parseLocationDirective(LocationStruct& loc, const std::string& directive);
		void _parseLocationRoot(LocationStruct& loc, const std::string& value);
		void _parseLocationAutoindex(LocationStruct& loc, const std::string& value);
		void _parseLocationMethod(LocationStruct& loc, const std::string& value);
		void _parseLocationIndexPage(LocationStruct& loc, const std::string& value);
		void _parseLocationReturn(LocationStruct& loc, const std::string& value);
		void _parseLocationCgiPath(LocationStruct& loc, const std::string& value);
		void _parseLocationCgiExt(LocationStruct& loc, const std::string& value);
		void _parseLocationUploadEnabled(LocationStruct& loc, const std::string& value);
		void _parseLocationUploadPath(LocationStruct& loc, const std::string& value);
		bool _isValidHost(const std::string& host) const;
		bool _isValidIPv4(const std::string& ip) const;
		bool _isValidDomain(const std::string& domain) const;
		size_t _parseBodySize(const std::string& sizeStr) const;
		bool _isValidLocationPath(const std::string& path) const;
    	void _initializeLocationStruct(LocationStruct& loc) const;

	public:
		SingleServerConfig(std::string server, ConfigStruct *conf);
		class NoListenException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};
		class DublicateRootException : public std::exception 
		{
			public : 
				virtual const char * what () const throw ();
		};
		class InvalidPathException: public std::exception
		{
			public:
				virtual const char * what () const throw ();
		};
		class DuplicateServerNameException: public std::exception
		{
			public :
				virtual const char * what () const throw ();
		};
		
};
void printConfigStruct(const ConfigStruct &conf);
                     

#endif