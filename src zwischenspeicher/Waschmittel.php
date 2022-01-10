<?php

$dbname = 'washingmachine';
$dbuser = 'root';  
$dbpass = ''; 
$dbhost = 'localhost'; 

$connect = @mysqli_connect($dbhost,$dbuser,$dbpass,$dbname);

if(!$connect){
	exit();
}

$Waschmittel = $_GET["waschmittel"];

$query = "INSERT INTO waschmittel (value) VALUES ('$Waschmittel')";
$result = mysqli_query($connect,$query);

?>