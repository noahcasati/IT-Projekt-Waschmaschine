<?php

$dbname = 'IT-Projekt-DB';
$dbuser = 'admin';  
$dbpass = 'admin'; 
$dbhost = 'localhost'; 

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);
// Check connection
if ($conn->connect_error) {
  die("Connection failed: " . $conn->connect_error);
} 

$sql = "INSERT INTO heizung (state, time) VALUES ('$_GET[heizung]', CURRENT_TIMESTAMP)";
$sql = "INSERT INTO pumpe (state, time) VALUES ('$_GET[pumpe]', CURRENT_TIMESTAMP)";
$sql = "INSERT INTO rpm (value, time) VALUES ('$_GET[rpm]', CURRENT_TIMESTAMP)";
//$sql = "UPDATE steuerung SET abpumpen, time, ) VALUES ('$_GET[pumpe]', CURRENT_TIMESTAMP) WHERE id";
$sql = "INSERT INTO temperature (value, time) VALUES ('$_GET[temperature]', CURRENT_TIMESTAMP)";
$sql = "INSERT INTO tuer (state, time) VALUES ('$_GET[tuer]', CURRENT_TIMESTAMP)";
$sql = "INSERT INTO waschmittel (value, time) VALUES ('$_GET[waschmittel]', CURRENT_TIMESTAMP)";
$sql = "INSERT INTO waterlevel (value, time) VALUES ('$_GET[waterlevel]', CURRENT_TIMESTAMP)";
$sql = "INSERT INTO watervalve (state, time) VALUES ('$_GET[watervalve]', CURRENT_TIMESTAMP)";

if ($conn->query($sql) === TRUE) {
  echo "New record created successfully";
} else {
  echo "Error: " . $sql . "<br>" . $conn->error;
}

$conn->close();

echo "klappt\n\n";

echo $_GET["heizung"];

?>