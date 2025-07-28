#ifndef RESPONDERROR_HPP
# define RESPONDERROR_HPP

# include "./parsing_request.hpp"


class RespondError : public ParsingRequest
{
private:

public:
    void ResERROR(const int ErrorStat);
    const std::string GenerateResErr(const int ErrorStat);
};

#endif