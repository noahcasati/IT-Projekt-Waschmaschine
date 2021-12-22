<?php

$dbname = 'washingmachine';
$dbuser = 'root';  
$dbpass = ''; 
$dbhost = 'localhost'; 

$connect = @mysqli_connect($dbhost,$dbuser,$dbpass,$dbname);

if(!$connect){
	exit();
}

//$temperature = $_GET["temperature"];

$query = "INSERT INTO temperature (Temperature) VALUES ('$temperature')";
$result = mysqli_query($connect,$query);

?>