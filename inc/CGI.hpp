#ifndef CGI_HPP
#define CGI_HPP


#include "parsing_request.hpp"



class CGI : public ParsingRequest
{

    private:
        std::map<std::string, std::string> env_vars;
        std::string script_path;
        std::string path_info;      // PATH_INFO for CGI
        std::string query_string;   // QUERY_STRING for CGI
        int cgi_fd; // File descriptor for the CGI process
        pid_t cgi_pid; // Process ID of the CGI process
        std::string output_buffer; // Buffer to store CGI output
        int status; // Status of the CGI execution
        int is_cgi; // Flag to indicate if the request is a CGI request
        int error_code; // Error code for CGI errors
        std::string error_message; // Error message for CGI errors

    public:
        CGI();
        ~CGI();
        bool set_env_var(std::map<std::string, std::string>& env_vars, const ParsingRequest& request);
        bool execute(std::map<std::string, std::string>& env_vars);
        bool execute_with_body(std::map<std::string, std::string>& env_vars, const std::string& body_data);
        bool read_output(); // Read output from CGI process
        std::string get_output() const { return output_buffer; }
        int get_cgi_fd() const { return cgi_fd; }
        pid_t get_cgi_pid() const { return cgi_pid; }
        int get_status() const { return status; }
        void close_cgi();
        std::string get_output_buffer() const { return output_buffer; }
        std::string get_script_path() const { return script_path; }
        std::string get_path_info() const { return path_info; }
        std::string get_query_string() const { return query_string; }
        std::map<std::string, std::string> get_env_vars() const { return env_vars; }
        bool check_is_cgi(const ParsingRequest &request);
        void handleCGI(ParsingRequest *parser);
        int get_error_code() const { return error_code; }
        std::string get_error_message() const { return error_message; }
        
    private:
        std::string get_interpreter(const std::string& script_path);
        bool wait_with_timeout(int timeout_seconds);
        bool send_post_data(int fd, const std::string& body_data);
};














#endif