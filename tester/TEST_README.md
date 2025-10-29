# Web Server Test Suite

A comprehensive test script for the webserver that covers edge cases and various HTTP request scenarios.

## Requirements

- Python 3.x
- curl
- Running webserver on 127.0.0.1:1080

## Usage

### Start your webserver first:
```bash
./webserv config/file.conf
```

### In another terminal, run the test suite:
```bash
./test_webserver.py
```

Or with Python explicitly:
```bash
python3 test_webserver.py
```

## Test Categories

### URL-Encoded POST Tests
- ✓ Basic URL-encoded data with multiple parameters
- ✓ Empty values in URL-encoded data
- ✓ Plain text without = sign (edge case that was causing crashes)
- ✓ Multiple consecutive ampersands
- ✓ Keys without values
- ✓ Special characters in values

### Malformed Data Tests
- ✓ Malformed URL-encoded data (multiple equals)
- ✓ Only equals signs
- ✓ Only ampersands
- ✓ Mixed separators
- ✓ Trailing ampersands
- ✓ Leading ampersands

### Content-Length Tests
- ✓ Exact Content-Length header
- ✓ Empty body (Content-Length: 0)
- ✓ Large payload (1MB)

### HTTP Method Tests
- ✓ Basic GET requests
- ✓ Non-existent resources (404)
- ✓ Invalid HTTP methods
- ✓ HEAD requests
- ✓ DELETE requests
- ✓ OPTIONS requests

### Header Tests
- ✓ Missing Content-Type header
- ✓ Multiple conflicting headers
- ✓ Connection: keep-alive
- ✓ Connection: close

### Special Cases
- ✓ Unicode in URL-encoded data
- ✓ Timeout handling
- ✓ Large payload handling

## Sample Output

```
============================================================
Web Server Comprehensive Test Suite
============================================================

→ Testing server at http://127.0.0.1:1080

[TEST] POST with basic URL-encoded data
→ Command: curl -s -w ...
✓ Status: 200

[TEST] POST with plain text (no = sign) - Edge case
→ Command: curl -s -w ...
✓ Status: 200 (No crash)

...

============================================================
Test Summary
============================================================
Passed: 28
Failed: 0
Total:  28
```

## Configuration

Edit the following variables at the top of the script to test different servers:

```python
HOST = "127.0.0.1"
PORT = 1080
BASE_URL = f"http://{HOST}:{PORT}"
TIMEOUT = 5
```

## Edge Cases Tested

1. **Empty values** - Keys with = but no value
2. **Missing delimiters** - Plain text without separators
3. **Multiple delimiters** - Consecutive ampersands or equals
4. **Boundary conditions** - Leading/trailing separators
5. **Special characters** - URL-encoded unicode
6. **Large payloads** - 1MB of data
7. **Content-Length edge cases** - Zero length, exact match
8. **Invalid methods** - Methods not in HTTP spec
9. **Missing headers** - Requests without required headers
10. **Connection management** - keep-alive and close

## Bug Found and Fixed

The original bug that caused the segmentation fault:
- **Issue**: POST with plain text (e.g., "hello") without the `=` sign would crash
- **Root cause**: `handle_url_encoded()` function tried to access `pairs[1]` when the split operation only returned `pairs[0]`
- **Fix**: Added bounds checking to verify the pair has both key and value before accessing

```python
# Test case that was crashing the server:
curl -X POST http://127.0.0.1:1080/ -H "Content-Length: 5" -d "hello" -v
```

After the fix, this now returns a proper response or 400 error instead of crashing.
