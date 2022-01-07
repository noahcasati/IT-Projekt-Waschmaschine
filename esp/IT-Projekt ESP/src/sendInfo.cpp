void SendInfo() { 
if (client.connect(192.168.0.104, 3306))
 {
 Serial.println("Connected, sending data...");
 client.print("GET phpmyadmin/InfoPushing/");
 delay(500);
 client.print(phpfile);
 delay(500);
 client.print(valuename);
 delay(500);
 client.print(value);
 delay(500);
 client.println(" HTTP/1.1"); 
 client.println("Host: 192.168.0.104"); 
 client.println("Connection: close");
 client.println();
 client.println();
 delay (2000);
 client.stop();
 Serial.println("Finished sending data");
}
else
{ Serial.println("Connection has failed\n"); }
delay (3000);
}
****************************************************
char phpfile = "Temperature.php?";
char valuename = "temperature=";
int value = dtemperature;
SendInfo();

****************************************************
char phpfile= "Heizung.php?";
char valuename="heizung=";
int value=dheizung;
SendInfo();
***************************************************
char phpfile="rpm.php?";
char valuename="rpm=";
int value=drpm;
SendInfo();
***************************************************
char phpfile="Waschmittel.php?";
char valuename="waschmittel=";
int value=dwashmittel;
SendInfo();
***************************************************
char phpfile="Waterlevel.php?";
char valuename="waterlevel=";
int value=dwaterlevel;
SendInfo();
***************************************************
char phpfile="Waterstate.php?";
char valuename="waterstate=";
int value=dwaterstate;
SendInfo();
*****************************************************
char phpfile="Pumpe.php?";
char valuename="pumpe=";
int value=dpumpe;
SendInfo();
******************************************************

