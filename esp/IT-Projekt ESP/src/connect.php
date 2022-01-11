<?php

$dbname = 'IT-Projekt-DB';
$dbuser = 'admin';  
$dbpass = 'admin'; 
$dbhost = 'http://192.168.0.228/'; 

$connect = @mysqli_connect($dbhost,$dbuser,$dbpass,$dbname);

if(!$connect){
	echo "Error: " . mysqli_connect_error();
	exit();
}

echo "Connection Success!<br><br>";

$temp = $_GET["temperature"];
$door = ;
$heizung =;
$pumpe =;
$watervalve =;

int rpm = 0;
int waschmittel = 0;

double temp = 0.0;
double waterlevel = 0.0;


$query = "INSERT INTO IT-Projekt-DB.heizung (state) VALUES ('$heizung')";
$query = "INSERT INTO IT-Projekt-DB.pumpe (state) VALUES ('$pumpe')";
$result = mysqli_query($connect,$query);

echo "Insertion Success!<br>";

?>