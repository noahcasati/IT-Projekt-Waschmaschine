<?php

$dbname = 'IT-Projekt-DB';
$dbuser = 'admin';  
$dbpass = 'admin'; 
$dbhost = 'localhost'; 

// Create connection
$conn = new mysqli($dbhost, $dbuser, $dbpass, $dbname);
// Check connection
if ($conn->connect_error) {
  die("Connection failed: " . $conn->connect_error);
} 

$sql = "UPDATE steuerung SET abpumpen = 0, prog1 = 0, prog2 = 0, prog3 = 3 WHERE id = 1;";

if ($conn->query($sql) === TRUE) {
  echo "New record created successfully";
} else {
  echo "Error: " . $sql . "<br>" . $conn->error;
}

$conn->close();

echo "klappt\n\n";

//echo $_GET["heizung"];

?>