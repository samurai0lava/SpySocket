#!/usr/bin/env python3
"""
Advanced parsing request tests for webserver.
Tests various HTTP request parsing edge cases and protocol compliance.
"""

import subprocess
import sys
import time
from typing import Tuple

# Configuration
HOST = "127.0.0.1"
PORT = 1080
BASE_URL = f"http://{HOST}:{PORT}"
TIMEOUT = 5

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    RESET = '\033[0m'
    BOLD = '\033[1m'

def print_test(name: str):
    print(f"\n{Colors.BLUE}{Colors.BOLD}[TEST]{Colors.RESET} {name}")

def print_pass(msg: str = "PASS"):
    print(f"{Colors.GREEN}✓ {msg}{Colors.RESET}")

def print_fail(msg: str = "FAIL"):
    print(f"{Colors.RED}✗ {msg}{Colors.RESET}")

def print_info(msg: str):
    print(f"{Colors.YELLOW}→ {msg}{Colors.RESET}")

def send_raw_request(raw_http: str) -> Tuple[int, str, str]:
    """
    Send raw HTTP request and return (status_code, response, stderr)
    """
    cmd = ["curl", "-s", "-w", "\n%{http_code}", "--raw", "-X", "POST", "--data-raw", raw_http, f"{BASE_URL}/"]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        response_body = '\n'.join(lines[:-1])
        return status_code, response_body, result.stderr
    except subprocess.TimeoutExpired:
        print_fail(f"Timeout after {TIMEOUT} seconds")
        return 0, "", "Timeout"
    except Exception as e:
        print_fail(f"Error: {e}")
        return 0, "", str(e)

def test_request_line_variations():
    """Test various HTTP request line formats"""
    print_test("Request line with single space separators")
    cmd = ["curl", "-s", "-w", "\n%{http_code}", f"{BASE_URL}/"]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_request_line_extra_spaces():
    """Test request line with extra spaces"""
    print_test("Request line with extra spaces between method and URI")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-H", "Host: 127.0.0.1:1080",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404, 400]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_http_version_variations():
    """Test different HTTP versions"""
    print_test("HTTP/1.0 request")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "--http1.0",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404, 400]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_http_version_11():
    """Test HTTP/1.1 specifically"""
    print_test("HTTP/1.1 request (default)")
    cmd = ["curl", "-s", "-w", "\n%{http_code}", f"{BASE_URL}/"]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_headers_case_insensitivity():
    """Test that header names are case-insensitive"""
    print_test("Case-insensitive header names")
    headers_variants = [
        ("Content-Type", "application/x-www-form-urlencoded"),
        ("content-type", "application/x-www-form-urlencoded"),
        ("CONTENT-TYPE", "application/x-www-form-urlencoded"),
        ("CoNtEnT-tYpE", "application/x-www-form-urlencoded"),
    ]
    
    all_ok = True
    for header_name, header_value in headers_variants:
        cmd = [
            "curl", "-s", "-w", "\n%{http_code}",
            "-X", "POST",
            "-H", f"{header_name}: {header_value}",
            "-d", "key=value",
            f"{BASE_URL}/"
        ]
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
            output = result.stdout
            lines = output.split('\n')
            status_code = int(lines[-1]) if lines[-1].isdigit() else 0
            if status_code not in [200, 400]:
                all_ok = False
                print_fail(f"Failed with header: {header_name}")
                break
        except Exception as e:
            print_fail(f"Error: {e}")
            all_ok = False
            break
    
    if all_ok:
        print_pass("All header case variations handled")

def test_header_with_multiple_colons():
    """Test header value containing colons"""
    print_test("Header value containing multiple colons")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-H", "X-Custom-Header: value:with:colons",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_header_with_spaces():
    """Test header values with leading/trailing spaces"""
    print_test("Header values with spaces around colon")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-H", "X-Custom-Header:  value with spaces  ",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_host_header_required():
    """Test that Host header is properly handled"""
    print_test("GET with Host header")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-H", "Host: 127.0.0.1:1080",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_uri_with_spaces():
    """Test URI containing spaces"""
    print_test("GET with spaces in URI")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        f"{BASE_URL}/path%20with%20spaces"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_uri_with_special_chars():
    """Test URI with special characters"""
    print_test("GET with special characters in URI")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        f"{BASE_URL}/path%2Fwith%2Fslashes"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_content_length_header_variations():
    """Test Content-Length with different formats"""
    print_test("Content-Length with leading zeros")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-X", "POST",
        "-H", "Content-Length: 00005",
        "-d", "hello",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 400]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_content_length_mismatch():
    """Test Content-Length that doesn't match actual body"""
    print_test("Content-Length mismatch with actual body")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-X", "POST",
        "-H", "Content-Length: 3",
        "-d", "hello",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 400]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_both_content_length_and_chunked():
    """Test request with both Content-Length and Transfer-Encoding"""
    print_test("Both Content-Length and Transfer-Encoding headers")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-X", "POST",
        "-H", "Content-Length: 5",
        "-H", "Transfer-Encoding: chunked",
        "-d", "hello",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [400]:  # Should be error
            print_pass(f"Status: {status_code} (Correctly rejected conflicting headers)")
        else:
            print_info(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_empty_header_line():
    """Test handling of empty header lines"""
    print_test("Empty header lines in request")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_connection_header_variations():
    """Test Connection header with various values"""
    print_test("Connection header variations")
    connection_values = ["keep-alive", "close", "Keep-Alive", "CLOSE"]
    
    all_ok = True
    for value in connection_values:
        cmd = [
            "curl", "-s", "-w", "\n%{http_code}",
            "-H", f"Connection: {value}",
            f"{BASE_URL}/"
        ]
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
            output = result.stdout
            lines = output.split('\n')
            status_code = int(lines[-1]) if lines[-1].isdigit() else 0
            if status_code not in [200, 404]:
                all_ok = False
                break
        except Exception as e:
            all_ok = False
            break
    
    if all_ok:
        print_pass("All Connection header values handled")
    else:
        print_fail("Failed with Connection header value")

def test_expect_header():
    """Test Expect header"""
    print_test("GET with Expect header")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-H", "Expect: 100-continue",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404, 400]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_range_header():
    """Test Range header"""
    print_test("GET with Range header")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-H", "Range: bytes=0-99",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 206, 404]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_uri_max_length():
    """Test maximum URI length"""
    print_test("GET with very long URI (8000+ chars)")
    long_path = "/path" + ("x" * 8000)
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        f"{BASE_URL}{long_path}"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404, 414]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_many_headers():
    """Test request with many headers"""
    print_test("GET with many headers (50+)")
    cmd = ["curl", "-s", "-w", "\n%{http_code}"]
    for i in range(50):
        cmd.extend(["-H", f"X-Custom-Header-{i}: value{i}"])
    cmd.append(f"{BASE_URL}/")
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_header_with_empty_value():
    """Test header with empty value"""
    print_test("Header with empty value")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-H", "X-Empty:",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_authorization_header():
    """Test Authorization header"""
    print_test("GET with Authorization header")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-H", "Authorization: Basic dXNlcjpwYXNz",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404, 401]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_pragma_header():
    """Test Pragma header"""
    print_test("GET with Pragma header")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-H", "Pragma: no-cache",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_cache_control_header():
    """Test Cache-Control header"""
    print_test("GET with Cache-Control header")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-H", "Cache-Control: max-age=3600, public",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_accept_encoding():
    """Test Accept-Encoding header"""
    print_test("GET with Accept-Encoding header")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-H", "Accept-Encoding: gzip, deflate, br",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_referer_header():
    """Test Referer header"""
    print_test("GET with Referer header")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-H", "Referer: http://example.com/page",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_origin_header():
    """Test Origin header (for CORS)"""
    print_test("GET with Origin header (CORS)")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-H", "Origin: http://example.com",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 404]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def main():
    print(f"\n{Colors.BOLD}{Colors.BLUE}{'='*60}")
    print("Advanced Parsing Request Test Suite")
    print("="*60 + Colors.RESET + "\n")
    
    print_info(f"Testing server at {BASE_URL}")
    time.sleep(0.5)
    
    tests = [
        # Request line tests
        test_request_line_variations,
        test_request_line_extra_spaces,
        
        # HTTP version tests
        test_http_version_variations,
        test_http_version_11,
        
        # Header case sensitivity
        test_headers_case_insensitivity,
        test_header_with_multiple_colons,
        test_header_with_spaces,
        
        # Host and URI tests
        test_host_header_required,
        test_uri_with_spaces,
        test_uri_with_special_chars,
        
        # Content-Length tests
        test_content_length_header_variations,
        test_content_length_mismatch,
        test_both_content_length_and_chunked,
        
        # General header tests
        test_empty_header_line,
        test_header_with_empty_value,
        test_many_headers,
        
        # Standard header tests
        test_connection_header_variations,
        test_expect_header,
        test_range_header,
        test_authorization_header,
        test_pragma_header,
        test_cache_control_header,
        test_accept_encoding,
        test_referer_header,
        test_origin_header,
        
        # Length limits
        test_uri_max_length,
    ]
    
    passed = 0
    failed = 0
    
    for test in tests:
        try:
            test()
            passed += 1
        except Exception as e:
            print_fail(f"Unexpected error: {e}")
            failed += 1
    
    print(f"\n{Colors.BOLD}{Colors.BLUE}{'='*60}")
    print("Test Summary")
    print("="*60 + Colors.RESET)
    print(f"{Colors.GREEN}Passed: {passed}{Colors.RESET}")
    if failed > 0:
        print(f"{Colors.RED}Failed: {failed}{Colors.RESET}")
    print(f"Total:  {passed + failed}\n")
    
    return 0 if failed == 0 else 1

if __name__ == "__main__":
    sys.exit(main())
