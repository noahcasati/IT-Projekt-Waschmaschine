<?php

$dbname = 'washingmachine';
$dbuser = 'root';  
$dbpass = ''; 
$dbhost = 'localhost'; 

$connect = @mysqli_connect($dbhost,$dbuser,$dbpass,$dbname);

if(!$connect){
	exit();
}

$waterstate = $_GET["waterstate"];

$query = "INSERT INTO waterstate (state) VALUES ('$waterstate')";
$result = mysqli_query($connect,$query);

?>