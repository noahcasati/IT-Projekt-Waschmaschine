void initWiFi() 
{
 WiFi.mode(WIFI_STA);
 WiFi.begin(Name des WLAN, PassWort); 
 Serial.print("Connecting to WiFi ..");
 while (WiFi.status() != WL_CONNECTED)
 {
 Serial.print('.');
 delay(1000);
 }
 Serial.println(WiFi.localIP());
}
 
void setup() {
 Serial.begin(115200);
 initWiFi();
 Serial.print("RRSI: ");
 Serial.println(WiFi.RSSI());
}
void loop() {
 dtemperature = analogRead(34);
*******************************************
 if (client.connect(server, 1234))
 {
 Serial.println("Connected, sending data...");
 client.print("GET phpmyadmin/InfoPushing/temprecord.php?"); 
 client.print("Temperature=");
 client.print(dtemperature);
 client.println(" HTTP/1.1"); 
 client.println("Host: 192.168.0.104"); 
 client.println("Connection: close");
 client.println();
 client.println();
 delay (5000);
 client.stop();
 Serial.println("Finished sending data");
}
else
{ Serial.println("Connection has failed\n"); }
delay (3000);
**************************************************************
}