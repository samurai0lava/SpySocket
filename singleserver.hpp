
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
		void _parseKeyValue(std::string);
		void _setVariables(std::string config);
		void _handleLocation(std::string line);
		void _handleErrorPage(std::string line);
		LocationStruct _fillLocationStruct(std::string block);
	public:
		SingleServerConfig(std::string server, ConfigStruct *conf);
};
void printConfigStruct(const ConfigStruct &conf);
                     

#endif