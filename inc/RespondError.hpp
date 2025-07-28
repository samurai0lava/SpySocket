#ifndef RESPONDERROR_HPP
# define RESPONDERROR_HPP

# include "./parsing_request.hpp"
# include <iostream>
# include <string>

// Standalone functions for error handling
std::string getStatusPhrase(int errorCode);
std::string generateErrorPageHTML(int errorCode, const std::string& errorMessage);
std::string getDefaultErrorMessage(int errorCode);
void ResERROR(const int ErrorStat);
const std::string GenerateResErr(const int ErrorStat);


#endif