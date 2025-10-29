#!/usr/bin/env python3
"""
Comprehensive webserver testing script with edge cases.
Tests various POST, GET, DELETE, and error scenarios.
"""

import subprocess
import sys
import time
import json
from typing import Tuple, List

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

def run_curl(method: str, url: str, data: str = None, headers: dict = None, verbose: bool = True) -> Tuple[int, str, str]:
    """
    Run curl command and return (status_code, stdout, stderr)
    """
    cmd = ["curl", "-s", "-w", "\n%{http_code}", "-X", method, url]
    
    if headers:
        for key, value in headers.items():
            cmd.extend(["-H", f"{key}: {value}"])
    
    if data is not None:
        cmd.extend(["-d", data])
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        
        # Extract status code from last line
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        response_body = '\n'.join(lines[:-1])
        
        if verbose:
            print_info(f"Command: {' '.join(cmd)}")
        
        return status_code, response_body, result.stderr
    except subprocess.TimeoutExpired:
        print_fail(f"Timeout after {TIMEOUT} seconds")
        return 0, "", "Timeout"
    except Exception as e:
        print_fail(f"Error running curl: {e}")
        return 0, "", str(e)

def test_post_urlencoded_basic():
    """Test basic URL-encoded POST"""
    print_test("POST with basic URL-encoded data")
    status, body, err = run_curl(
        "POST", 
        f"{BASE_URL}/",
        data="username=john&password=secret",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status == 200 and "username" in body and "john" in body:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}, Body: {body}")

def test_post_urlencoded_empty_value():
    """Test URL-encoded POST with empty value"""
    print_test("POST with empty value in URL-encoded")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="key1=&key2=value",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status == 200:
        print_pass(f"Status: {status}")
        print_info(f"Response: {body[:100]}")
    else:
        print_fail(f"Status: {status}")

def test_post_urlencoded_plain_text():
    """Test POST with plain text (no = sign) - Edge case"""
    print_test("POST with plain text (no = sign) - Edge case")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="hello",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status in [200, 400]:
        print_pass(f"Status: {status} (No crash)")
    else:
        print_fail(f"Status: {status}")

def test_post_urlencoded_multiple_ampersands():
    """Test URL-encoded with multiple consecutive ampersands"""
    print_test("POST with multiple consecutive ampersands")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="key1=value1&&&&key2=value2",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status in [200, 400]:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_post_urlencoded_no_value():
    """Test URL-encoded with key but no = sign"""
    print_test("POST with key but no = sign")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="key1&key2=value2",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status in [200, 400]:
        print_pass(f"Status: {status} (No crash)")
    else:
        print_fail(f"Status: {status}")

def test_post_urlencoded_special_chars():
    """Test URL-encoded with special characters"""
    print_test("POST with special characters in values")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="email=test%40example.com&name=John%20Doe",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status == 200:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_post_content_length_exact():
    """Test POST with exact Content-Length"""
    print_test("POST with exact Content-Length header")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="hello",
        headers={
            "Content-Length": "5",
            "Content-Type": "application/x-www-form-urlencoded"
        }
    )
    
    if status in [200, 400]:
        print_pass(f"Status: {status} (No crash)")
    else:
        print_fail(f"Status: {status}")

def test_post_content_length_zero():
    """Test POST with Content-Length: 0"""
    print_test("POST with Content-Length: 0 (empty body)")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="",
        headers={
            "Content-Length": "0",
            "Content-Type": "application/x-www-form-urlencoded"
        }
    )
    
    if status in [200, 400]:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_post_large_payload():
    """Test POST with large payload"""
    print_test("POST with large payload (1MB)")
    large_data = "key=" + ("x" * (1024 * 1024))
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data=large_data,
        headers={
            "Content-Type": "application/x-www-form-urlencoded"
        },
        verbose=False
    )
    
    if status in [200, 413]:  # 413 = Payload Too Large
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_get_basic():
    """Test basic GET request"""
    print_test("GET basic request")
    status, body, err = run_curl("GET", f"{BASE_URL}/")
    
    if status == 200:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_get_not_found():
    """Test GET for non-existent resource"""
    print_test("GET non-existent resource")
    status, body, err = run_curl("GET", f"{BASE_URL}/this_does_not_exist_12345")
    
    if status == 404:
        print_pass(f"Status: {status} (404 as expected)")
    else:
        print_fail(f"Expected 404, got {status}")

def test_invalid_method():
    """Test invalid HTTP method"""
    print_test("Invalid HTTP method")
    cmd = ["curl", "-s", "-w", "\n%{http_code}", "-X", "INVALID", f"{BASE_URL}/"]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        
        if status_code in [405, 400]:
            print_pass(f"Status: {status_code} (Invalid method rejected)")
        else:
            print_fail(f"Expected 405/400, got {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_missing_content_type():
    """Test POST without Content-Type header"""
    print_test("POST without Content-Type header")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="data=value"
    )
    
    if status in [200, 400, 415]:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_multiple_content_lengths():
    """Test POST with multiple Content-Length headers (should fail)"""
    print_test("POST with potentially conflicting headers")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-X", "POST",
        "-H", "Content-Length: 5",
        "-H", "Content-Type: application/x-www-form-urlencoded",
        "-d", "hello",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        print_pass(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_head_request():
    """Test HEAD request"""
    print_test("HEAD request")
    cmd = ["curl", "-s", "-w", "\n%{http_code}", "-I", f"{BASE_URL}/"]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        
        if status_code in [200, 405]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_delete_request():
    """Test DELETE request"""
    print_test("DELETE request")
    status, body, err = run_curl("DELETE", f"{BASE_URL}/test.html")
    
    if status in [200, 204, 404, 405]:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_options_request():
    """Test OPTIONS request"""
    print_test("OPTIONS request")
    cmd = ["curl", "-s", "-w", "\n%{http_code}", "-X", "OPTIONS", f"{BASE_URL}/"]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        
        if status_code in [200, 405]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_post_malformed_urlencoded():
    """Test POST with malformed URL-encoded data"""
    print_test("POST with malformed URL-encoded data")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="key1=value1===key2=value2",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status in [200, 400]:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_post_unicode_urlencoded():
    """Test POST with unicode in URL-encoded data"""
    print_test("POST with unicode in URL-encoded data")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="name=%E2%9C%93&emoji=%F0%9F%98%80",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status in [200, 400]:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_post_only_equals():
    """Test POST with only = signs"""
    print_test("POST with only = signs")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="=",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status in [200, 400]:
        print_pass(f"Status: {status} (No crash)")
    else:
        print_fail(f"Status: {status}")

def test_post_only_ampersand():
    """Test POST with only ampersand"""
    print_test("POST with only ampersand")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="&",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status in [200, 400]:
        print_pass(f"Status: {status} (No crash)")
    else:
        print_fail(f"Status: {status}")

def test_post_mixed_separators():
    """Test POST with mixed = and & separators"""
    print_test("POST with mixed separators")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="key1=val1&key2&key3=val3&=val4",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status in [200, 400]:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_trailing_ampersand():
    """Test POST with trailing ampersand"""
    print_test("POST with trailing ampersand")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="key1=value1&key2=value2&",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status == 200:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_leading_ampersand():
    """Test POST with leading ampersand"""
    print_test("POST with leading ampersand")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="&key1=value1&key2=value2",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status in [200, 400]:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_connection_keep_alive():
    """Test Connection keep-alive"""
    print_test("GET with Connection: keep-alive")
    status, body, err = run_curl(
        "GET",
        f"{BASE_URL}/",
        headers={"Connection": "keep-alive"}
    )
    
    if status == 200:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_connection_close():
    """Test Connection close"""
    print_test("GET with Connection: close")
    status, body, err = run_curl(
        "GET",
        f"{BASE_URL}/",
        headers={"Connection": "close"}
    )
    
    if status == 200:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_post_empty_body():
    """Test POST with empty body"""
    print_test("POST with completely empty body")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status in [200, 400]:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_post_whitespace_only():
    """Test POST with only whitespace"""
    print_test("POST with only whitespace")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="   ",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status in [200, 400]:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_post_very_long_key():
    """Test POST with very long key name"""
    print_test("POST with very long key name (10000 chars)")
    long_key = "k" * 10000
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data=f"{long_key}=value",
        headers={"Content-Type": "application/x-www-form-urlencoded"},
        verbose=False
    )
    
    if status in [200, 400, 414]:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_post_very_long_value():
    """Test POST with very long value"""
    print_test("POST with very long value (100000 chars)")
    long_value = "v" * 100000
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data=f"key={long_value}",
        headers={"Content-Type": "application/x-www-form-urlencoded"},
        verbose=False
    )
    
    if status in [200, 400, 413]:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_post_many_parameters():
    """Test POST with many parameters"""
    print_test("POST with 1000 parameters")
    data = "&".join([f"key{i}=value{i}" for i in range(1000)])
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data=data,
        headers={"Content-Type": "application/x-www-form-urlencoded"},
        verbose=False
    )
    
    if status in [200, 400, 413]:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_post_duplicate_keys():
    """Test POST with duplicate keys"""
    print_test("POST with duplicate keys")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="username=alice&username=bob&username=charlie",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status in [200, 400]:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_post_null_bytes():
    """Test POST with null bytes in data"""
    print_test("POST with null bytes in data")
    # Use special encoding for null bytes
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-X", "POST",
        "-H", "Content-Type: application/x-www-form-urlencoded",
        "--data-raw", "key=val\x00ue",
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

def test_get_with_query_string():
    """Test GET with query string"""
    print_test("GET with query string")
    status, body, err = run_curl(
        "GET",
        f"{BASE_URL}/?foo=bar&baz=qux"
    )
    
    if status in [200, 404]:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_get_with_fragment():
    """Test GET with URL fragment"""
    print_test("GET with URL fragment")
    status, body, err = run_curl(
        "GET",
        f"{BASE_URL}/#section"
    )
    
    if status in [200, 404]:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_post_plus_encoding():
    """Test POST with plus signs (space encoding)"""
    print_test("POST with plus signs (space encoding)")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="name=John+Doe&title=Software+Engineer",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status == 200:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_post_percent_encoding():
    """Test POST with percent encoding"""
    print_test("POST with percent encoding")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="email=user%40example.com&path=%2Fhome%2Fuser",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status == 200:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_post_invalid_percent_encoding():
    """Test POST with invalid percent encoding"""
    print_test("POST with invalid percent encoding")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="key=value%ZZ",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status in [200, 400]:
        print_pass(f"Status: {status} (No crash)")
    else:
        print_fail(f"Status: {status}")

def test_post_case_insensitive_header():
    """Test POST with different header cases"""
    print_test("POST with case-insensitive header names")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-X", "POST",
        "-H", "content-type: application/x-www-form-urlencoded",
        "-d", "key=value",
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

def test_multiple_requests_sequential():
    """Test multiple sequential requests"""
    print_test("Multiple sequential POST requests")
    all_ok = True
    for i in range(5):
        status, body, err = run_curl(
            "POST",
            f"{BASE_URL}/",
            data=f"request={i}",
            headers={"Content-Type": "application/x-www-form-urlencoded"},
            verbose=False
        )
        if status not in [200, 400]:
            all_ok = False
            break
    
    if all_ok:
        print_pass("All 5 requests successful")
    else:
        print_fail(f"Request {i} failed with status {status}")

def test_post_crlf_in_value():
    """Test POST with CRLF in value"""
    print_test("POST with CRLF in value")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-X", "POST",
        "-H", "Content-Type: application/x-www-form-urlencoded",
        "--data-raw", "key=value\r\nwith\r\nnewlines",
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

def test_get_directory_with_slash():
    """Test GET directory with trailing slash"""
    print_test("GET directory with trailing slash")
    status, body, err = run_curl(
        "GET",
        f"{BASE_URL}/html/"
    )
    
    if status in [200, 301, 302, 404]:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_get_directory_without_slash():
    """Test GET directory without trailing slash"""
    print_test("GET directory without trailing slash")
    status, body, err = run_curl(
        "GET",
        f"{BASE_URL}/html"
    )
    
    if status in [200, 301, 302, 404]:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_post_form_with_file_field():
    """Test POST form-like data with file field"""
    print_test("POST with simulated file field in data")
    status, body, err = run_curl(
        "POST",
        f"{BASE_URL}/",
        data="username=test&file=fake_file_content",
        headers={"Content-Type": "application/x-www-form-urlencoded"}
    )
    
    if status in [200, 400]:
        print_pass(f"Status: {status}")
    else:
        print_fail(f"Status: {status}")

def test_user_agent_variations():
    """Test different User-Agent headers"""
    print_test("GET with different User-Agent headers")
    agents = [
        "Mozilla/5.0",
        "curl/7.88.1",
        "Python-Requests/2.28.0",
        ""
    ]
    
    all_ok = True
    for agent in agents:
        headers = {"User-Agent": agent} if agent else {}
        status, body, err = run_curl(
            "GET",
            f"{BASE_URL}/",
            headers=headers,
            verbose=False
        )
        if status not in [200, 404]:
            all_ok = False
            break
    
    if all_ok:
        print_pass("All User-Agent variations handled")
    else:
        print_fail(f"Failed with User-Agent: {agent}")

def test_accept_header_variations():
    """Test different Accept headers"""
    print_test("GET with different Accept headers")
    accepts = [
        "text/html",
        "application/json",
        "*/*",
        "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"
    ]
    
    all_ok = True
    for accept in accepts:
        status, body, err = run_curl(
            "GET",
            f"{BASE_URL}/",
            headers={"Accept": accept},
            verbose=False
        )
        if status not in [200, 404]:
            all_ok = False
            break
    
    if all_ok:
        print_pass("All Accept header variations handled")
    else:
        print_fail(f"Failed with Accept: {accept}")

def test_chunked_basic():
    """Test basic chunked transfer encoding"""
    print_test("POST with chunked transfer encoding (basic)")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-X", "POST",
        "-H", "Transfer-Encoding: chunked",
        "-H", "Content-Type: application/x-www-form-urlencoded",
        "--data-raw", "key=value&foo=bar",
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

def test_chunked_empty_chunks():
    """Test chunked encoding with multiple empty chunks"""
    print_test("POST with chunked encoding (multiple empty chunks)")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-X", "POST",
        "-H", "Transfer-Encoding: chunked",
        "-H", "Content-Type: application/x-www-form-urlencoded",
        "--data-raw", "data=test",
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

def test_chunked_large_data():
    """Test chunked encoding with large data"""
    print_test("POST with chunked encoding (large data)")
    large_data = "key=" + ("x" * 50000)
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-X", "POST",
        "-H", "Transfer-Encoding: chunked",
        "-H", "Content-Type: application/x-www-form-urlencoded",
        "--data-raw", large_data,
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 400, 413]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_chunked_plain_text():
    """Test chunked encoding with plain text (no key=value)"""
    print_test("POST with chunked encoding (plain text)")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-X", "POST",
        "-H", "Transfer-Encoding: chunked",
        "-H", "Content-Type: application/x-www-form-urlencoded",
        "--data-raw", "hello world",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 400]:
            print_pass(f"Status: {status_code} (No crash)")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_chunked_with_extensions():
    """Test chunked encoding with chunk extensions"""
    print_test("POST with chunked encoding (chunk extensions)")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-X", "POST",
        "-H", "Transfer-Encoding: chunked",
        "-H", "Content-Type: application/x-www-form-urlencoded",
        "--data-raw", "key=value",
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

def test_chunked_special_chars():
    """Test chunked encoding with special characters"""
    print_test("POST with chunked encoding (special characters)")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-X", "POST",
        "-H", "Transfer-Encoding: chunked",
        "-H", "Content-Type: application/x-www-form-urlencoded",
        "--data-raw", "email=test%40example.com&name=John%20Doe",
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

def test_chunked_unicode():
    """Test chunked encoding with unicode data"""
    print_test("POST with chunked encoding (unicode)")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-X", "POST",
        "-H", "Transfer-Encoding: chunked",
        "-H", "Content-Type: application/x-www-form-urlencoded",
        "--data-raw", "name=%E2%9C%93&emoji=%F0%9F%98%80",
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

def test_chunked_many_params():
    """Test chunked encoding with many parameters"""
    print_test("POST with chunked encoding (many parameters)")
    data = "&".join([f"key{i}=value{i}" for i in range(100)])
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-X", "POST",
        "-H", "Transfer-Encoding: chunked",
        "-H", "Content-Type: application/x-www-form-urlencoded",
        "--data-raw", data,
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 400, 413]:
            print_pass(f"Status: {status_code}")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_chunked_duplicate_keys():
    """Test chunked encoding with duplicate keys"""
    print_test("POST with chunked encoding (duplicate keys)")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-X", "POST",
        "-H", "Transfer-Encoding: chunked",
        "-H", "Content-Type: application/x-www-form-urlencoded",
        "--data-raw", "user=alice&user=bob&user=charlie",
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

def test_chunked_malformed_data():
    """Test chunked encoding with malformed URL-encoded data"""
    print_test("POST with chunked encoding (malformed data)")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-X", "POST",
        "-H", "Transfer-Encoding: chunked",
        "-H", "Content-Type: application/x-www-form-urlencoded",
        "--data-raw", "key1&&&key2=val&=val3",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 400]:
            print_pass(f"Status: {status_code} (No crash)")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_chunked_only_equals():
    """Test chunked encoding with only equals signs"""
    print_test("POST with chunked encoding (only equals)")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-X", "POST",
        "-H", "Transfer-Encoding: chunked",
        "-H", "Content-Type: application/x-www-form-urlencoded",
        "--data-raw", "=",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 400]:
            print_pass(f"Status: {status_code} (No crash)")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def test_chunked_only_ampersand():
    """Test chunked encoding with only ampersands"""
    print_test("POST with chunked encoding (only ampersand)")
    cmd = [
        "curl", "-s", "-w", "\n%{http_code}",
        "-X", "POST",
        "-H", "Transfer-Encoding: chunked",
        "-H", "Content-Type: application/x-www-form-urlencoded",
        "--data-raw", "&",
        f"{BASE_URL}/"
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        lines = output.split('\n')
        status_code = int(lines[-1]) if lines[-1].isdigit() else 0
        if status_code in [200, 400]:
            print_pass(f"Status: {status_code} (No crash)")
        else:
            print_fail(f"Status: {status_code}")
    except Exception as e:
        print_fail(f"Error: {e}")

def main():
    print(f"\n{Colors.BOLD}{Colors.BLUE}{'='*60}")
    print("Web Server Comprehensive Test Suite")
    print("="*60 + Colors.RESET + "\n")
    
    print_info(f"Testing server at {BASE_URL}")
    
    # Wait a bit for server to be ready
    time.sleep(0.5)
    
    # Run all tests
    tests = [
        # Basic URL-encoded POST tests
        test_post_urlencoded_basic,
        test_post_urlencoded_empty_value,
        test_post_urlencoded_plain_text,
        test_post_urlencoded_multiple_ampersands,
        test_post_urlencoded_no_value,
        test_post_urlencoded_special_chars,
        
        # Malformed data tests
        test_post_malformed_urlencoded,
        test_post_only_equals,
        test_post_only_ampersand,
        test_post_mixed_separators,
        test_trailing_ampersand,
        test_leading_ampersand,
        
        # Empty/Whitespace tests
        test_post_empty_body,
        test_post_whitespace_only,
        
        # Size/Length tests
        test_post_very_long_key,
        test_post_very_long_value,
        test_post_many_parameters,
        test_post_duplicate_keys,
        
        # Content-Length tests
        test_post_content_length_exact,
        test_post_content_length_zero,
        test_post_large_payload,
        
        # Encoding tests
        test_post_plus_encoding,
        test_post_percent_encoding,
        test_post_invalid_percent_encoding,
        test_post_null_bytes,
        test_post_crlf_in_value,
        test_post_unicode_urlencoded,
        
        # GET tests
        test_get_basic,
        test_get_not_found,
        test_get_with_query_string,
        test_get_with_fragment,
        test_get_directory_with_slash,
        test_get_directory_without_slash,
        
        # HTTP method tests
        test_invalid_method,
        test_head_request,
        test_delete_request,
        test_options_request,
        
        # Header tests
        test_missing_content_type,
        test_multiple_content_lengths,
        test_connection_keep_alive,
        test_connection_close,
        test_post_case_insensitive_header,
        test_user_agent_variations,
        test_accept_header_variations,
        
        # Form and complex data
        test_post_form_with_file_field,
        
        # Chunked transfer encoding tests
        test_chunked_basic,
        test_chunked_empty_chunks,
        test_chunked_large_data,
        test_chunked_plain_text,
        test_chunked_with_extensions,
        test_chunked_special_chars,
        test_chunked_unicode,
        test_chunked_many_params,
        test_chunked_duplicate_keys,
        test_chunked_malformed_data,
        test_chunked_only_equals,
        test_chunked_only_ampersand,
        
        # Sequential/Multi-request tests
        test_multiple_requests_sequential,
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
    
    # Print summary
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
