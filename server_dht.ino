/*
 * Copyright (c) 2018, circuits4you.com
 * All rights reserved.
 * Create a TCP Server on ESP8266 NodeMCU. 
 * TCP Socket Server Send Receive Demo
*/

#include <ESP8266WiFi.h>

#define SendKey 0  //Button to send data Flash BTN on NodeMCU

int port = 8888;  //Port number
WiFiServer server(port);

//Server connect to WiFi Network
const char *ssid = "NETGEAR37-2";  //Enter your wifi SSID
const char *password = "grandcurtain880";  //Enter your wifi Password

int count=0;
float temp = 70.7;
#include "DHT.h"

// Pin
#define DHTPIN 5

// Use DHT11 sensor
#define DHTTYPE DHT11

// Initialize DHT sensor
//DHT dht(DHTPIN, DHTTYPE, 15);
DHT dht(DHTPIN, DHTTYPE);

//=======================================================================
//                    Power on setup
//=======================================================================
void setup() 
{
  Serial.begin(115200);
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); //Connect to wifi
 
  // Wait for connection  
  Serial.println("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED) {   
    delay(500);
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  
  server.begin();
  Serial.print(WiFi.localIP());
  Serial.print(" on port ");
  Serial.println(port);
  // Init DHT
  dht.begin();

  
}
//=======================================================================
//                    Loop
//=======================================================================

void loop() 
{
  WiFiClient client = server.available();
  
  if (client) {
    if(client.connected())
    {
      Serial.println("Client Connected");
    }
    
    while(client.connected()){      
      while(client.available()>0){
        // read data from the connected client
        Serial.write(client.read()); 
      }
      
      //Send Data to connected client
      count++;
      if (count==1){
        float h = dht.readHumidity(); 
        float t = dht.readTemperature(); 
        char str[80];
        sprintf(str, "%f,%f", t*9.0 / 5.0 + 32.0,h);
       
       
        
        client.print(str);  
      }  
      while(Serial.available()>0)
      {
        Serial.println("sending data back to clien");
        client.write(Serial.read());
      }
       
    }
    client.stop();
    Serial.println(count);
    count =0;
    Serial.println("Client disconnected");    
  }
}
