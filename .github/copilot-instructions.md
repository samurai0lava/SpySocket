# Webserv-42 AI Coding Instructions

## Project Overview

Webserv-42 is a C++98 HTTP server implementation following the 42 School standards. It supports static file serving, CGI execution, and HTTP methods (GET, POST, DELETE) with nginx-style configuration parsing.

## Architecture Components

### Core Server Structure

- **Servers class**: Singleton managing multiple server instances with epoll-based event handling
- **Config class**: Parses nginx-style `.conf` files into `ConfigStruct` and `LocationStruct` objects
- **CClient class**: Handles individual client connections, inherits from `ParsingRequest`
- **MethodHandler**: Routes HTTP methods to appropriate handlers (GET, POST, DELETE)

### Key Data Flow

1. `main.cpp` → `Config::StartToSet()` → `Servers::getInstance()` → `epollFds()`
2. Client connection → `CClient` creation → method routing → response generation
3. CGI requests follow: request parsing → environment setup → process execution → output handling

## Configuration System

### Config File Structure (nginx-style)

```nginx
server {
    listen 8080;
    server_name localhost;
    root html/;

    location /cgi-bin {
        cgi_path /usr/bin/python3;
        cgi_ext .py;
        method GET POST;
    }
}
```

### Key Structs

- `ConfigStruct`: Server-level config (listen ports, server_name, root, error_pages)
- `LocationStruct`: Location-specific rules (methods, CGI settings, autoindex, uploads)

## CGI Implementation

### CGI Detection

- Files in `cgi-bin/` directories are treated as CGI scripts
- Extension matching via `cgi_ext` config directive
- Interpreter detection from file extension or shebang

### CGI Execution Pattern

```cpp
CGI cgi;
cgi.set_env_var(env_vars, request);
cgi.execute_with_body(env_vars, body_data);
// Non-blocking read with timeout handling
cgi.read_output();
```

### Environment Variables

Standard CGI variables: `REQUEST_METHOD`, `QUERY_STRING`, `CONTENT_TYPE`, `CONTENT_LENGTH`, `PATH_INFO`, `SCRIPT_NAME`

## HTTP Method Handling

### Method Router Pattern

`MethodHandler.cpp` creates `CClient` instances which handle all methods through:

- `HandleAllMethod()` for regular HTTP operations
- `HandleCGIMethod()` for CGI script execution

### Location Matching

Uses `getClosest()` algorithm in `POST.cpp` to find best-matching location block for URIs.

## Build and Development

### Build System

```bash
make          # Compile with C++98 flags
./webserv config/config.conf
```

### File Organization

- `src/`: Implementation files organized by feature (Methods/, POST/, CGI/, utils/)
- `inc/`: Header files for internal components
- `include/`: Public interfaces (Config.hpp, server.hpp, etc.)
- `config/`: Sample configuration files
- `www/`: Web root with static files and CGI scripts

## Key Patterns

### Memory Management

- Manual memory management (C++98)
- RAII pattern for file descriptors and sockets
- Singleton pattern for `Servers` class with explicit cleanup

### Error Handling

- Custom exception classes: `FileOpenException`, `ServerInsideServerException`
- HTTP error responses via `RespondError.hpp`
- CGI timeout handling with process cleanup

### Networking

- Epoll-based I/O multiplexing
- Non-blocking socket operations
- Buffer management with `READ_SIZE` (64KB) chunks

## Development Guidelines

### Adding New Features

1. Check location matching in `match_location.cpp` for routing
2. Extend `LocationStruct` for new config directives
3. Add parsing logic in `singleserver.cpp`
4. Implement feature in appropriate method handler

### Testing CGI

- Place scripts in `www/cgi-bin/`
- Ensure proper shebang lines
- Test with different content types and methods
- Verify environment variable passing

### Configuration Testing

- Validate config syntax before server start
- Test location precedence with overlapping paths
- Verify error page handling for different status codes
