#!/usr/bin/env python3

import cgi
import cgitb
import os
import sys

# Enable CGI error reporting
cgitb.enable()

# Ensure output is unbuffered
sys.stdout.reconfigure(line_buffering=True)

# Print content type header with proper line ending
print("Content-Type: text/html")
print("")  # Empty line required between headers and body
sys.stdout.flush()

try:
    # Get form data
    form = cgi.FieldStorage()
    feedback = form.getvalue("uname", "")
    
    if feedback:
        feedback = feedback.strip()
    
    if feedback:
        # Save feedback to uploads directory
        uploads_dir = os.path.join(os.path.dirname(os.path.dirname(__file__)), "uploads")
        feedback_file = os.path.join(uploads_dir, "feedback.txt")
        
        # Create uploads directory if it doesn't exist
        os.makedirs(uploads_dir, exist_ok=True)
        
        # Save feedback
        with open(feedback_file, "a", encoding="utf-8") as f:
            f.write(f"Feedback: {feedback}\n---\n")
        
        # Success response
        response = """<!DOCTYPE html>
<html lang='en'>
<head>
  <meta charset='UTF-8'>
  <title>Feedback Received</title>
  <style>
    body {
      font-family: 'Roboto Mono', monospace;
      background: #ffefe4;
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 100vh;
      margin: 0;
    }
    .success-container {
      background: white;
      padding: 40px;
      border-radius: 20px;
      box-shadow: 0 8px 25px rgba(0, 0, 0, 0.1);
      text-align: center;
      max-width: 500px;
    }
    .success-title {
      color: #4CAF50;
      font-size: 2rem;
      margin-bottom: 20px;
    }
    .success-message {
      color: #666;
      line-height: 1.6;
      margin-bottom: 30px;
    }
    .back-btn {
      background: #4a90e2;
      color: white;
      padding: 12px 24px;
      border: none;
      border-radius: 8px;
      text-decoration: none;
      display: inline-block;
      transition: background 0.3s ease;
    }
    .back-btn:hover {
      background: #357abd;
    }
  </style>
</head>
<body>
  <div class='success-container'>
    <h2 class='success-title'>✓ Thank You!</h2>
    <p class='success-message'>Your feedback has been received successfully. We appreciate you taking the time to help us improve SpySocket.</p>
    <a href='/html/main-page.html' class='back-btn'>Back to Main Page</a>
  </div>
</body>
</html>"""
        print(response)
        
    else:
        # No feedback provided
        response = """<!DOCTYPE html>
<html lang='en'>
<head>
  <meta charset='UTF-8'>
  <title>No Feedback</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; padding: 50px; }
    .error-container { background: #f8f9fa; padding: 30px; border-radius: 10px; display: inline-block; }
  </style>
</head>
<body>
  <div class='error-container'>
    <h2>No Feedback Received</h2>
    <p>Please provide your feedback.</p>
    <a href='/html/feedback-page.html'>Go Back</a>
  </div>
</body>
</html>"""
        print(response)

except Exception as e:
    # Error response
    response = f"""<!DOCTYPE html>
<html lang='en'>
<head>
  <meta charset='UTF-8'>
  <title>Error</title>
  <style>
    body {{ font-family: Arial, sans-serif; text-align: center; padding: 50px; }}
    .error-container {{ background: #f8d7da; padding: 30px; border-radius: 10px; display: inline-block; }}
  </style>
</head>
<body>
  <div class='error-container'>
    <h2>Error Processing Feedback</h2>
    <p>Sorry, there was an error: {str(e)}</p>
    <a href='/html/feedback-page.html'>Go Back</a>
  </div>
</body>
</html>"""
    print(response)

# Ensure all output is flushed
sys.stdout.flush()
