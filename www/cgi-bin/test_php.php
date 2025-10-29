#!/usr/bin/php
<?php
header("Content-Type: text/html");
?>
<!DOCTYPE html>
<html>
<head>
    <title>PHP CGI Test</title>
</head>
<body>
    <h1>PHP is Working!</h1>
    <p><strong>PHP Version:</strong> <?php echo phpversion(); ?></p>
    <p><strong>Server Time:</strong> <?php echo date('Y-m-d H:i:s'); ?></p>

    <h2>Server Variables</h2>
    <pre><?php print_r($_SERVER); ?></pre>
</body>
</html>
