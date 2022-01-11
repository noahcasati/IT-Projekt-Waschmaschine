#include "ESP8266WiFi.h"
#include "SoftwareSerial.h"
#include "Metro.h"

char* ssid     = "UPCC434158";
char* password = "xxbj5wpkmzcA";
char* host = "192.168.0.228";

SoftwareSerial esp_uno (3 , 5); // RX, TX

Metro sekunde = Metro(1000);

Metro alive = Metro(3000);

//Serial buffer
String Data = "";

//Input variables
bool heizung = true;
bool pumpe = true;
bool tuer = true;

bool watervalve = true;

int rpm = 0;
int waschmittel = 0;

double temperature = 0.0;
double waterlevel = 0.0;


//Controll variables
bool abpumpen = false;
bool wp1 = false;
bool wp2 = false;
bool wp3 = false;


bool aliveSignal = false;
  

void setup()
{
  
    Serial.begin(9600);
    esp_uno.begin(9600);

    pinMode(LED_BUILTIN, OUTPUT);

    // Connect Wifi
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    delay(50);
    esp_uno.write("test test 123");               // send to esp via SoftwareSerial


    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    delay(50);
}



void loop()
{
    /*!
      esp_uno.write("test test 123");               // send to Arduino via SoftwareSerial
      delay(50);

      if(sekunde.check())
      {
        Serial.println("loop läuft mit serial.println");
        
      }

      if(alive.check())
      {
        digitalWrite(LED_BUILTIN, aliveSignal);
        aliveSignal = !aliveSignal;
      }
      
      delay(50);
    */

/*!
 while (esp_uno.available())
    {
        char character = esp_uno.read(); // Receive a single character from the software serial port
        Data.concat(character); // Add the received character to the receive buffer
        if (character == '\n')
        {
            Serial.print("Received: ");
            Serial.println(Data);

            // Add your code to parse the received line here....

            // Clear receive buffer so we're ready to receive the next line
            Data = "";
        }
    }
*/
///*!  

  
    Serial.println("Heizung: ");
    Serial.print(heizung);
    Serial.println("Pumpe");
    Serial.print(pumpe);
    Serial.println("rpm: ");
    Serial.print(rpm);
    Serial.println("Temperatur: ");
    Serial.print(temperature);
    Serial.println("Tuer");
    Serial.print(tuer);
    Serial.println("Waschmittel: ");
    Serial.print(waschmittel);
    Serial.println("Waterlevel: ");
    Serial.print(waterlevel);
    Serial.println("Waterventil: ");
    Serial.print(watervalve);

    delay(3000);
  
   
    Serial.print("connecting to ");
    Serial.println(host);

    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
    }

 


    // This will send the request to the server
 client.print(String("GET connect.php?") + ("&heizung=") + heizung + ("&pumpe=") + pumpe + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 1000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }

    // Read all the lines of the reply from server and print them to Serial
    while(client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
        
    }

    Serial.println();
    Serial.println("closing connection");

    //*/


}