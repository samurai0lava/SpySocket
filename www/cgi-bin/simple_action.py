#!/usr/bin/env python3

import sys
import os
import urllib.parse

print("Content-Type: text/html")
print("")

try:
    # Get content length
    content_length = os.environ.get('CONTENT_LENGTH', '0')
    if content_length.isdigit():
        content_length = int(content_length)
    else:
        content_length = 0
    
    # Read POST data directly
    post_data = ""
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
    
    # Parse form data manually
    feedback = ""
    if post_data:
        # Parse URL-encoded data
        parsed_data = urllib.parse.parse_qs(post_data)
        if 'uname' in parsed_data:
            feedback = parsed_data['uname'][0].strip()
    
    if feedback:
        # Save feedback
        uploads_dir = "/home/iouhssei/Desktop/Webserv-42/www/uploads"
        feedback_file = os.path.join(uploads_dir, "feedback.txt")
        
        # Create directory if needed
        os.makedirs(uploads_dir, exist_ok=True)
        
        # Write feedback
        with open(feedback_file, "a", encoding="utf-8") as f:
            f.write(f"Feedback: {feedback}\n---\n")
        
        print("""<!DOCTYPE html>
<html>
<head><title>Success</title></head>
<body>
    <h2>Thank You!</h2>
    <p>Your feedback has been received: """ + feedback + """</p>
    <a href="/html/main-page.html">Back to Main</a>
</body>
</html>""")
    else:
        print("""<!DOCTYPE html>
<html>
<head><title>No Feedback</title></head>
<body>
    <h2>No Feedback Received</h2>
    <p>Please provide feedback.</p>
    <p>POST Data received: """ + post_data + """</p>
    <a href="/html/feedback-page.html">Go Back</a>
</body>
</html>""")

except Exception as e:
    print(f"""<!DOCTYPE html>
<html>
<head><title>Error</title></head>
<body>
    <h2>Error</h2>
    <p>Error: {str(e)}</p>
</body>
</html>""")

sys.stdout.flush()
