#ifndef LOGS_HPP
#define LOGS_HPP

#include "parsing_request.hpp"

std::string generate_access_line(ParsingRequest& request);
std::string generate_error_line(int error_code, const std::string& error_message);
void access_log(ParsingRequest& request);
void access_error(int error_code, const std::string& error_message);

#endif