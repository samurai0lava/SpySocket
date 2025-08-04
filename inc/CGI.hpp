#ifndef CGI_HPP
#define CGI_HPP


#include "parsing_request.hpp"



class CGI : public ParsingRequest
{

    private:
        std::map<std::string, std::string> env_vars;
        std::string script_path;
        int cgi_fd; // File descriptor for the CGI process
        pid_t cgi_pid; // Process ID of the CGI process
        std::string output_buffer; // Buffer to store CGI output
        int status;

    public:
        CGI();
        ~CGI();
        bool set_env_var(std::map<std::string, std::string>& env_vars, const ParsingRequest& request);
        bool execute(std::map<std::string, std::string>& env_vars);
        bool read_output(); // Read output from CGI process
        std::string get_output() const { return output_buffer; }
        int get_cgi_fd() const { return cgi_fd; }
        pid_t get_cgi_pid() const { return cgi_pid; }
        int get_status() const { return status; }
        void close_cgi();
        std::string get_output_buffer() const { return output_buffer; }
        std::map<std::string, std::string> get_env_vars() const { return env_vars; }
        bool read_output();
        void close_cgi();
};












#endif