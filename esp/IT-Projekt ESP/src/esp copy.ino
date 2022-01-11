#include "ESP8266WiFi.h"
#include <ESP8266HTTPClient.h>
#include "SoftwareSerial.h"
#include "Metro.h"

char *ssid = "UPCC434158";
char *password = "xxbj5wpkmzcA";
char *host = "192.168.0.228";

SoftwareSerial esp_uno(2, 3); // RX, TX

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

char * msgIn = "";
char * datensatz;

void setup()
{

    Serial.begin(9600);
    esp_uno.begin(9600);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(2, INPUT);
    pinMode(3, OUTPUT);

    // Connect Wifi
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    delay(50);

    esp_uno.write("test test 123"); // send to esp via SoftwareSerial

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
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
    HTTPClient sender;

    const int httpPort = 80;

    if (!client.connect(host, httpPort))
    {
        Serial.println("connection failed");
        return;
    }

    if (sender.begin(client, String("http://192.168.0.228/connect.php?heizung=") + heizung + ("&pumpe=") + pumpe + ("&rpm=") + rpm + ("&temperature=") + temperature + ("&tuer=") + tuer + ("&waschmittel=") + waschmittel + ("&waterlevel=") + waterlevel + ("&watervalve=") + watervalve))
    {
         // Response speichern
        int httpCode = sender.GET();
            if (httpCode >0)
            {
                if (httpCode == HTTP_CODE_OK)
                {
                    // Webserver Antwort
                    String payload = sender.getString();
                    Serial.println(payload);
                }
            }
            else
            {
                Serial.printf("HTTP-Error: ", sender.errorToString(httpCode).c_str());
            }

            // Verbindung beenden
            sender.end();
    }
    else
    {
        Serial.printf("HTTP-Verbindung konnte nicht hergestellt werden!");
    }

    esp_uno.print("10");
    esp_uno.println("\n");
    delay(30);

    while(esp_uno.available() > 0)
  {
    char incoming = esp_uno.read();
    if(esp_uno.read() == '\n')
    {
      Serial.println(incoming);
      msgIn = incoming;
    }
    delay(30);
  }

  datensatz = strtok(msgIn, ";");

}