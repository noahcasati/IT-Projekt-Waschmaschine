<?php

$dbname = 'washingmachine';
$dbuser = 'root';  
$dbpass = ''; 
$dbhost = 'localhost'; 

$connect = @mysqli_connect($dbhost,$dbuser,$dbpass,$dbname);

if(!$connect){
	exit();
}

$rpm = $_GET["rpm"];

$query = "INSERT INTO rpm (value) VALUES ('$rpm')";
$result = mysqli_query($connect,$query);

?>