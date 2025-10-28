#!/usr/bin/php

<?php
// Simple PHP test script

// Display PHP information
echo "<h1>PHP Test Script</h1>";
echo "<p>PHP Version: " . phpversion() . "</p>";
echo "<p>Current Date/Time: " . date('Y-m-d H:i:s') . "</p>";

// Test environment variables
echo "<h2>Environment Variables</h2>";
echo "<pre>";
echo "SERVER_SOFTWARE: " . ($_SERVER['SERVER_SOFTWARE'] ?? 'N/A') . "\n";
echo "SERVER_NAME: " . ($_SERVER['SERVER_NAME'] ?? 'N/A') . "\n";
echo "REQUEST_METHOD: " . ($_SERVER['REQUEST_METHOD'] ?? 'N/A') . "\n";
echo "QUERY_STRING: " . ($_SERVER['QUERY_STRING'] ?? 'N/A') . "\n";
echo "</pre>";

// Test simple calculation
echo "<h2>Test Calculation</h2>";
echo "<p>2 + 2 = " . (2 + 2) . "</p>";

echo "<p>Script executed successfully!</p>";
?>
