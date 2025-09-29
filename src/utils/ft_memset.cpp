#include "../../include/server.hpp"

void ft_memset(void *b, int c, size_t len)
{
    unsigned char *ptr = static_cast<unsigned char*>(b);
    while (len-- > 0)
        *ptr++ = static_cast<unsigned char>(c);
}