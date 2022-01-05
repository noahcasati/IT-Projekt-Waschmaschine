<?php

$dbname = 'washingmachine';
$dbuser = 'root';  
$dbpass = ''; 
$dbhost = 'localhost'; 

$connect = @mysqli_connect($dbhost,$dbuser,$dbpass,$dbname);

if(!$connect){
	exit();
}

$waterlevel = $_GET["waterlevel"];

$query = "INSERT INTO waterlevel (waterlevel) VALUES ('$waterlevel')";
$result = mysqli_query($connect,$query);

?>