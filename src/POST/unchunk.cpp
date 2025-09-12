#include "../../inc/webserv.hpp"

int	hex_to_dec(string hex)
{
	int		res;
	string	hexa;
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
		if (hexa.find(hex[i]) == string::npos)
			return (-1);
		res += hexa.find(tolower(hex[i])) * pow(16, j++);
	}
	return (res);
}

// string malformed_request()
// {
//     string body = "<html><body><h1>400 Bad Request</h1><p>Malformed request.</p></body></html>";
//     string response =
//     "HTTP/1.1 400 Bad Request\r\n"
//     "Content-Type: text/html\r\n"
//     "Content-Length: " + to_string(body.size()) + "\r\n"
//     "\r\n" + body;
//     return (response);
// }

string	unchunk_data(char *chunk, size_t chunk_size)
{
	int				new_request;
	size_t			body_start;
	int				x;
	static string	filename;
	size_t			pos;
	fstream			file;
	size_t			chars_to_read;
	size_t			eol;
	string			size_hex;
	static bool		chunked;
	static int		img;

	// cout << "************\n";
	// cout << chunk;
	// cout << "******END******\n";
	string stringged_chunk(chunk, chunk_size);
	new_request = 0;
	body_start = 0;
	x = 0;
	// STEP 1: detect if this is a new HTTP request
	if (stringged_chunk.substr(0,
			stringged_chunk.find("\r\n")).find("HTTP") != string::npos)
	{
		pos = stringged_chunk.find("\r\n\r\n");
		if (pos == string::npos)
		{
			return ("error_400");
		}
		if (stringged_chunk.find("Transfer-Encoding: chunked") != string::npos)
			chunked = true;
		else
			chunked = false;
		if (stringged_chunk.find("Content-Type: image/png") != string::npos)
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
	// STEP 2: isolate just the body
	stringged_chunk = stringged_chunk.substr(body_start + x);
	chunk_size -= body_start + x;
	// STEP 3: prepare output file
	// cout << "NEW REQUEST : " << new_request << endl;
	// cout << "--> " << filename << endl;
	if (new_request)
		file.open(filename.c_str(), ios::out | ios::binary); // new file
	else
		file.open(filename.c_str(), ios::binary | ios::out | ios::app);
	// append
	if (!chunked)
	{
		// Handle unchunked data
		file.write(stringged_chunk.data(), chunk_size);
		// return (filename);
	}
	else
	{
		chars_to_read = 0;
		// STEP 4: process chunks
		while (chunk_size > 0)
		{
			// (a) find the CRLF that ends the size line
			eol = stringged_chunk.find("\r\n");
			if (eol == string::npos)
			{
				return ("error_400");
			}
			// (b) extract hex size and convert
			size_hex = stringged_chunk.substr(0, eol);
			// cout << "HEX : " << size_hex << endl;
			chars_to_read = hex_to_dec(size_hex);
			// (c) advance past the size line + CRLF
			stringged_chunk = stringged_chunk.substr(eol + 2);
			chunk_size -= (eol + 2);
			if (chars_to_read == 0)
			{
				// this is the terminating "0\r\n\r\n"
				break ;
			}
			// (d) check we actually have enough bytes left
			// cout << "CHARS TO READ : " << chars_to_read << endl;
			// cout << "CHUNK SIZE : " << chunk_size << endl;
			if (chars_to_read + 2 > chunk_size)
			{ // +2 for the trailing CRLF
				return ("error_400");
			}
			// (e) write exactly `chars_to_read` bytes
			file.write(stringged_chunk.data(), chars_to_read);
			// (f) advance past the data + trailing CRLF
			stringged_chunk = stringged_chunk.substr(chars_to_read + 2);
			chunk_size -= (chars_to_read + 2);
		}
	}
	return (filename);
}

// string	refactor_data(const char *buffer, size_t buffer_size)
// {
// 	string data(buffer, buffer_size);
//     size_t start_line_end = data.find("\r\n");
//     if(start_line_end == string::npos)
//         return ("error_400"); //malformed request
//     // static int new_request;
//     static string headers;
//     static string res;
// 	// static string body;

// 	if (data.substr(0, start_line_end).find("HTTP") != string::npos)
		//headers found
// 	{
//         cout << "NEW REQUEST!!!!!!\n";
//         res = "";
// 		size_t headers_end = data.find("\r\n\r\n");
// 		if (headers_end == string::npos)
// 			return ("error_400");
//         headers = data.substr(0, headers_end + 4);
// 		// cout << "--->HEADERS<---\n";
// 		// cout << headers;
// 		// cout << "DATA SIZE : " << data.length() << " HEADERS SIZE : " << headers.length() << endl;
// 		// cout << "--><--\n";
//         data = data.substr(headers_end + 4);
// 		// cout << "****DATA****\n";
// 		// write(1, data.data(), data.length());
// 		// cout << "************\n";
// 		buffer_size -= headers.length();
// 		// cout << "BUFFER SIZE : " << buffer_size << endl;
// 	}

//     if(headers.find("Transfer-Encoding") != string::npos)
//     {
// 		// cout << "-*-*-*-CHUNKED-*-*-*-\n";
//         int chars_to_read = 0;
//         size_t eol;

// 		// STEP 4: process chunks
// 		while (buffer_size > 0)
// 		{
// 			// (a) find the CRLF that ends the size line
// 			eol = data.find("\r\n");
// 			if (eol == string::npos)
// 				return ("error_400");
// 			// (b) extract hex size and convert
// 			chars_to_read = hex_to_dec(data.substr(0, eol));
// 			// (c) advance past the size line + CRLF
// 			data = data.substr(eol + 2);
// 			buffer_size -= (eol + 2);
// 			if (chars_to_read == 0)
// 			{
// 				// this is the terminating "0\r\n\r\n"
// 				break ;
// 			}
// 			// (d) check we actually have enough bytes left
// 			if (chars_to_read + 2 > (int)buffer_size)
// 			{ // +2 for the trailing CRLF
// 				return ("error_400");
// 			}
// 			// (e) add exactly `chars_to_read` bytes
//             res += data.substr(0, chars_to_read);
// 			// (f) advance past the data + trailing CRLF
// 			data = data.substr(chars_to_read + 2);
// 			buffer_size -= (chars_to_read + 2);
// 		}
//     }
//     else
// 	{
// 		// cout << "-*-*-*-UNCHUNKED-*-*-*-\n";
// 		//binary data comes separated
// 		cout << "****RES****\n";
// 		write(1, res.data(), res.length());
// 		cout << "*****RES_END******\n";
// 		res += buffer;
// 		cout << "***RES+BUFFER***\n";
// 		write(1, res.data(), res.length());
// 		cout << "***RES+BUFFER_END***\n";
//         return (res);
// 	}
// 	//maybe there's \r\n after the 0 check this later
//     return (headers + res);
// }

// void	refactor_data(string& buffer, const char* data, size_t len)
// {
// 	string data_recv(data, len);
// 	// cout << "***RECV****\n";
// 	// write(1, data_recv.data(), data_recv.length());
// 	// cout << "****RECV_END****\n";
//     size_t start_line_end = data_recv.find("\r\n");
//     static string headers;
//     string res;

// 	// cout << "LEN : " << len << endl;

// 	if (data_recv.substr(0, start_line_end).find("HTTP") != string::npos)
		//headers found
// 	{
//         // cout << "NEW REQUEST!!!!!!\n";
// 		buffer = "";
//         res = "";
// 		size_t headers_end = data_recv.find("\r\n\r\n");
//         headers = data_recv.substr(0, headers_end + 4);
//         data_recv = data_recv.substr(headers_end + 4);
// 		len -= headers.length();
// 		buffer.append(headers);
// 	}

// 	// cout << "LEN : " << len << endl;

//     if(headers.find("Transfer-Encoding") != string::npos)
//     {
// 		size_t chars_to_read = 0;
//         size_t eol;

// 		// STEP 4: process chunks
// 		while (len > 0)
// 		{
// 			// (a) find the CRLF that ends the size line
// 			eol = data_recv.find("\r\n");
// 			// (b) extract hex size and convert
// 			cout << "--> HEX STRINGGED : " << data_recv.substr(0,
				// eol) << " <--\n";
// 			chars_to_read = hex_to_dec(data_recv.substr(0, eol));
// 			// (c) advance past the size line + CRLF
// 			data_recv = data_recv.substr(eol + 2);
// 			len -= (eol + 2);
// 			if (chars_to_read == 0)
// 			{
// 				// this is the terminating "0\r\n\r\n"
// 				cout << "WwWWwwwWWWwwWWWWWWwwwWWwwwwwW\n";
// 				break ;
// 			}
// 			// (d) check we actually have enough bytes left
// 			// cout << "CHARS TO READ : " << chars_to_read << endl;
// 			// cout << "DATA SIZE : " << data_recv.length() << endl;
// 			if(chars_to_read > len)
// 			{
// 				cout << "LEN : " << len << " CHARS_TO_READ : " << chars_to_read << endl;
// 				cerr << "Not enough data sent!!\n";
// 				return ;
// 			}
// 			// (e) add exactly `chars_to_read` bytes
//             res += data_recv.substr(0, chars_to_read);
// 			// (f) advance past the data + trailing CRLF

// 			data_recv = data_recv.substr(chars_to_read + 2);
// 			// cout << "ALLOOOOOOOOOO\n";
// 			len -= (chars_to_read + 2);
// 		}
// 		buffer.append(res);
//     }
//     else
// 	{
// 		// cout << "***RES_BEFORE***\n";
// 		// write(1, res.data(), res.length());
// 		res += data_recv;
// 		// cout << "***RES_AFTER***\n";
// 		// write(1, res.data(), res.length());
// 		buffer.append(res);
// 		// cout << "***BUFFER***\n";
// 		// write(1, buffer.data(), buffer.length());
// 		// cout << "****BUFFER_END****\n";
// 	}
// 	//maybe there's \r\n after the 0 check this later
// }

void reset_refactor_data_state()
{
	// Reset by calling refactor_data with a special reset marker
	string dummy_buffer;
	refactor_data(dummy_buffer, NULL, 0);
}

bool is_chunked_transfer_complete()
{
	// Use a special call to query the status
	string dummy_buffer;
	refactor_data(dummy_buffer, NULL, (size_t)-1);
	return dummy_buffer == "complete";
}

void	refactor_data(string &buffer, const char *data, size_t len)
{
	static size_t	current_chunk_size = 0;
	static bool		reading_size = true;
	static bool		chunked_complete = false;
	size_t			headers_end;
	size_t			eol;
	size_t			end_marker;

	static string headers;
	static string chunk_buffer;
	
	// reset static variables
	if (data == NULL && len == 0) {
		current_chunk_size = 0;
		reading_size = true;
		chunked_complete = false;
		headers.clear();
		chunk_buffer.clear();
		return;
	}
	
	// query completion status
	if (data == NULL && len == (size_t)-1) {
		buffer = chunked_complete ? "complete" : "incomplete";
		return;
	}
	// Reset static variables if we detect start of a new request
	if (len > 0 && data[0] >= 'A' && data[0] <= 'Z' && headers.empty() && chunk_buffer.empty()) {
		current_chunk_size = 0;
		reading_size = true;
		headers.clear();
		chunk_buffer.clear();
	}
	try {
		chunk_buffer.append(data, len);
	}
	catch (std::bad_alloc& e) {
		std::cerr << "Memory allocation failed in refactor_data: " << e.what() << std::endl;
		return;
	}

	// Detect headers first only once at the beginning
	if (headers.empty())
	{
		headers_end = chunk_buffer.find("\r\n\r\n");
		if (headers_end == string::npos)
			return ; // wait for full headers
		headers = chunk_buffer.substr(0, headers_end + 4);
		try{
			buffer.append(headers);
		}
		catch (std::bad_alloc& e) {
			std::cerr << "Memory allocation failed in refactor_data: " << e.what() << std::endl;
			return;
		}
		chunk_buffer.erase(0, headers_end + 4);
	}

	if (headers.find("Transfer-Encoding") != string::npos && headers.find("chunked") != string::npos)
	{
		cout << "=== PROCESSING CHUNKED DATA ===" << endl;
		while (true)
		{
			if (reading_size)
			{
				eol = chunk_buffer.find("\r\n");
				if (eol == string::npos)
				{
					cout << "=== WAITING FOR CHUNK SIZE LINE ===" << endl;
					return ; // not enough data yet still waiting for size line
				}
				string hex_str = chunk_buffer.substr(0, eol);
				current_chunk_size = hex_to_dec(hex_str);
				cout << "CHUNK SIZE: " << current_chunk_size << " (hex: " << hex_str << ")" << endl;
				chunk_buffer.erase(0, eol + 2); // remove size line
				if (current_chunk_size == 0)
				{
					cout << "///////////EEEEEEEEEEEEEEEEEEEEEEEEEND//////////////////\n";
					chunked_complete = true;
					// End of chunks: expect final "\r\n"
					end_marker = chunk_buffer.find("\r\n");
					if (end_marker != string::npos)
						chunk_buffer.erase(0, end_marker + 2);
					
				// Check if there's any remaining data that might be a new request
					if (!chunk_buffer.empty()) {
						cout << "=== LEFTOVER DATA AFTER CHUNKS (" << chunk_buffer.size() << " bytes) ===" << endl;
						try{
						buffer.append(chunk_buffer);

						}
						catch (std::bad_alloc& e) {
							std::cerr << "Memory allocation failed in refactor_data: 000" << e.what() << std::endl;
							return;
						}

					}
					headers.clear();
					chunk_buffer.clear();
					current_chunk_size = 0;
					reading_size = true;
					return ;
				}
				reading_size = false;
			}
			if (chunk_buffer.size() < current_chunk_size + 2)
			{
				cout << "=== WAITING FOR FULL CHUNK (" << chunk_buffer.size() << "/" << (current_chunk_size + 2) << ") ===" << endl;
				return ;
			}
			try {
				buffer.append(chunk_buffer, 0, current_chunk_size);
				chunk_buffer.erase(0, current_chunk_size + 2);
				reading_size = true;
			}
			catch (std::bad_alloc& e) {
				std::cerr << "Memory allocation failed in refactor_data: " << e.what() << std::endl;
				return;
			}
		}
	}
	else
	{
		//none chunked or headers not complete yet

		try {
			buffer.append(chunk_buffer);
		}
		catch (std::bad_alloc& e) {
			std::cerr << "Memory allocation failed in refactor_data: " << e.what() << std::endl;
			return;
		}
		chunk_buffer.clear();
		if (!headers.empty() && headers.find("Content-Length:") != string::npos)
		{
			int content_length = atoi(headers.substr(headers.find("Content-Length") + strlen("Content-Length: ")).c_str());
			if(buffer.size() >= content_length + headers.length())
			{
				headers.clear();
				chunk_buffer.clear();
				current_chunk_size = 0;
				reading_size = true;
			}
		}
	}
}
