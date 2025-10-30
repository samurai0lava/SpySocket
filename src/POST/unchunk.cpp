#include "../../inc/webserv.hpp"

int	hex_to_dec(std::string hex)
{
	int		res;
	std::string	hexa;
	int		j;

	res = 0;
	hexa = "0123456789abcdef";
	j = 0;
	if (hex.empty())
		return (-1);
	for (int i = hex.length() - 1; i >= 0; i--)
	{
		if (hex[i] == ' ' || hex[i] == '\t')
			continue ;
		if (hexa.find(hex[i]) == std::string::npos)
			return (-1);
		res += hexa.find(tolower(hex[i])) * pow(16, j++);
	}
	return (res);
}

std::string	unchunk_data(char *chunk, size_t chunk_size)
{
	int				new_request;
	size_t			body_start;
	int				x;
	static std::string	filename;
	size_t			pos;
	std::fstream		file;
	size_t			chars_to_read;
	size_t			eol;
	std::string		size_hex;
	static bool		chunked;
	static int		img;

	std::string stringged_chunk(chunk, chunk_size);
	new_request = 0;
	body_start = 0;
	x = 0;
	if (stringged_chunk.substr(0,
			stringged_chunk.find("\r\n")).find("HTTP") != std::string::npos)
	{
		pos = stringged_chunk.find("\r\n\r\n");
		if (pos == std::string::npos)
		{
			return ("error_400");
		}
		if (stringged_chunk.find("Transfer-Encoding: chunked") != std::string::npos)
			chunked = true;
		else
			chunked = false;
		if (stringged_chunk.find("Content-Type: image/png") != std::string::npos)
		{
			img = 1;
		}
		else
		{
			img = 0;
		}
		new_request = 1;
		body_start = pos;
		x = 4; // skip \r\n\r\n
		if (img)
			filename = generate_filename("image_", ".png");
		else if (chunked)
			filename = generate_filename("chunk_", "");
		else
			filename = generate_filename("normal_", "");
	}
	if (body_start + x > stringged_chunk.size())
	{
		return ("error_400");
	}
	stringged_chunk = stringged_chunk.substr(body_start + x);
	chunk_size -= body_start + x;
	if (new_request)
		file.open(filename.c_str(), std::ios::out | std::ios::binary); // new file
	else
		file.open(filename.c_str(), std::ios::binary | std::ios::out | std::ios::app);
	if (!chunked)
	{
		file.write(stringged_chunk.data(), chunk_size);
	}
	else
	{
		chars_to_read = 0;
		while (chunk_size > 0)
		{
			eol = stringged_chunk.find("\r\n");
			if (eol == std::string::npos)
			{
				return ("error_400");
			}
			size_hex = stringged_chunk.substr(0, eol);
			chars_to_read = hex_to_dec(size_hex);
			stringged_chunk = stringged_chunk.substr(eol + 2);
			chunk_size -= (eol + 2);
			if (chars_to_read == 0)
			{
				break ;
			}
			if (chars_to_read + 2 > chunk_size)
			{
				return ("error_400");
			}
			file.write(stringged_chunk.data(), chars_to_read);
			stringged_chunk = stringged_chunk.substr(chars_to_read + 2);
			chunk_size -= (chars_to_read + 2);
		}
	}
	return (filename);
}

void reset_refactor_data_state()
{
	std::string dummy_buffer;
	refactor_data(dummy_buffer, NULL, 0);
}

bool is_chunked_transfer_complete()
{
	std::string dummy_buffer;
	refactor_data(dummy_buffer, NULL, (size_t)-1);
	return dummy_buffer == "complete";
}
bool refactor_data(std::string& buffer, const char* data, size_t len)
{
    static size_t	current_chunk_size = 0;
    static bool		reading_size = true;
    static bool		chunked_complete = false;
    size_t			headers_end;
    size_t			eol;
    size_t			end_marker;

    static std::string headers;
    static std::string chunk_buffer;

    if (data == NULL && len == 0) {
        current_chunk_size = 0;
        reading_size = true;
        chunked_complete = false;
        headers.clear();
        chunk_buffer.clear();
        return true;
    }

    if (data == NULL && len == (size_t)-1) {
        buffer = chunked_complete ? "complete" : "incomplete";
        return chunked_complete;
    }

    if (len > 0 && data[0] >= 'A' && data[0] <= 'Z' && headers.empty() && chunk_buffer.empty()) {
        current_chunk_size = 0;
        reading_size = true;
        headers.clear();
        chunk_buffer.clear();
    }

    try {
        chunk_buffer.append(data, len);
    }
    catch (std::exception& e) {
        access_error(500, "Internal Server Error: Memory allocation failed while appending data to chunk_buffer");
        std::cerr << "Memory allocation failed in refactor_data: 1 " << e.what() << std::endl;
        return false;
    }
    if (headers.empty())
    {
        headers_end = chunk_buffer.find("\r\n\r\n");
        if (headers_end == std::string::npos)
            return false;
        headers = chunk_buffer.substr(0, headers_end + 4);
        chunk_buffer.erase(0, headers_end + 4);
    }

    if (headers.find("Transfer-Encoding") != std::string::npos && headers.find("chunked") != std::string::npos)
    {
        while (true)
        {
            if (reading_size)
            {
                eol = chunk_buffer.find("\r\n");
                if (eol == std::string::npos)
                {
                    return false;
                }
                std::string hex_str = chunk_buffer.substr(0, eol);
                current_chunk_size = hex_to_dec(hex_str);
                chunk_buffer.erase(0, eol + 2);
                if (current_chunk_size == 0)
                {
                    chunked_complete = true;
                    end_marker = chunk_buffer.find("\r\n");
                    if (end_marker != std::string::npos)
                        chunk_buffer.erase(0, end_marker + 2);

                    if (!chunk_buffer.empty()) {
                        try{
                            buffer.append(chunk_buffer);
                        }
                        catch (std::exception& e) {
                            access_error(500, "Internal Server Error: Memory allocation failed while appending leftover chunk_buffer to buffer");
                            std::cerr << "Memory allocation failed in refactor_data: 000" << e.what() << std::endl;
                            return false;
                        }
                    }
                    headers.clear();
                    chunk_buffer.clear();
                    current_chunk_size = 0;
                    reading_size = true;
                    return true;
                }
                reading_size = false;
            }
            if (chunk_buffer.size() < current_chunk_size + 2)
            {
                return false;
            }
            try {
                buffer.append(chunk_buffer, 0, current_chunk_size);
                chunk_buffer.erase(0, current_chunk_size + 2);
                reading_size = true;
            }
            catch (std::exception& e) {
                access_error(500, "Internal Server Error: Memory allocation failed while appending chunk data to buffer");
                std::cerr << "Memory allocation failed in refactor_data: 3" << e.what() << std::endl;
                return false;
            }
        }
    }
    else
    {
        try {
            buffer.append(chunk_buffer);
        }
        catch (std::exception& e) {
            access_error(500, "Internal Server Error: Memory allocation failed while appending non-chunked data to buffer");
            std::cerr << "Memory allocation failed in refactor_data: 4" << e.what() << std::endl;
            return false;
        }
        chunk_buffer.clear();
        if (!headers.empty() && headers.find("Content-Length:") != std::string::npos)
        {
            int content_length = atoi(headers.substr(headers.find("Content-Length") + strlen("Content-Length: ")).c_str());
            if(buffer.size() >= content_length + headers.length())
            {
                headers.clear();
                chunk_buffer.clear();
                current_chunk_size = 0;
                reading_size = true;
                return true;
            }
        }
        return true;
    }
}
