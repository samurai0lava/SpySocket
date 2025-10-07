#!/usr/bin/php
<?php
// action_page.php

// Get feedback safely (with fallback)
$feedback = isset($_POST['uname']) ? trim($_POST['uname']) : '';

// If feedback not empty, save it
if (!empty($feedback)) {
    // Choose a writable location for your server
    $file = __DIR__ . '/../uploads/feedback.txt';

    // Append feedback to file
    file_put_contents($file, "Feedback: " . $feedback . "\n---\n", FILE_APPEND | LOCK_EX);

    // Send response back
    echo "<!DOCTYPE html>
    <html lang='en'>
    <head>
      <meta charset='UTF-8'>
      <title>Feedback Received</title>
      <link rel='stylesheet' href='style.css'>
    </head>
    <body class='delete-page-body'>
      <h1>Thank You!</h1>
      <p>Your feedback has been saved.</p>
      <div class='feedback-box'>
        <strong>Your message:</strong>
        <p>" . htmlspecialchars($feedback, ENT_QUOTES, 'UTF-8') . "</p>
      </div>
      <a href='main-page.html'>Back to main page</a>
    </body>
    </html>";
} else {
    // If no feedback provided
    echo "<p>No feedback received.</p>";
}
?>
