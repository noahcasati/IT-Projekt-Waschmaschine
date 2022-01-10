<?php

$dbname = 'washingmachine';
$dbuser = 'root';  
$dbpass = ''; 
$dbhost = 'localhost'; 

$connect = @mysqli_connect($dbhost,$dbuser,$dbpass,$dbname);

if(!$connect){
	exit();
}

$Pumpe = $_GET["pumpe"];

$query = "INSERT INTO pumpe (state) VALUES ('$Pumpe')";
$result = mysqli_query($connect,$query);

?>