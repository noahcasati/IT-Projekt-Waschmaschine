<?php

$dbname = 'washingmachine';
$dbuser = 'root';  
$dbpass = ''; 
$dbhost = 'localhost'; 

$connect = @mysqli_connect($dbhost,$dbuser,$dbpass,$dbname);

if(!$connect){
	exit();
}

$Heizung = $_GET["heizung"];

$query = "INSERT INTO Heizung (state) VALUES ('$Heizung')";
$result = mysqli_query($connect,$query);

?>