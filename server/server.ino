/*
 * Create a TCP Server on ESP8266 NodeMCU. 
 * TCP Socket Server Send Receive
*/
#include <Wire.h>
#include <ESP8266WiFi.h>
int port = 8888;  //Port number
const char* host = "10.0.0.208";

WiFiServer server(port);
//Server connect to WiFi Network
const char *ssid = "NETGEAR37-2";  //Enter your wifi SSID
const char *password = "grandcurtain880";  //Enter your wifi Password
int Index=0;
char Buffer[40];
#include "DHT.h"
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor
int connect2Raspberry(char *msg);

int connect2Raspberry(char*msg) {
  
    WiFiClient client;
    if (!client.connect(host, 1233)) {
      Serial.println("connection failed");
      delay(5000);
      return 0 ;
    }
    
   // Serial.println("sending data to server");
   // This will send a string to the server
   if (client.connected()) {
     client.printf("ESP8266: %s",msg);
   }
   String line = client.readStringUntil('\n');
   Serial.println(line);

   Serial.println("closing connection");
   client.stop();
   return 0;
    
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); //Connect to wifi

  //used for debug
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
 
  // Wait for connection  
  Serial.println("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED){
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

void loop() {

  WiFiClient client = server.available();
  if (client) {
    if(client.connected())  
      Serial.print(client.remoteIP());
      Serial.println("  Client Connected to Server");
      
    while(client.connected()){      
      while(client.available()>0){
        // read data from the connected client
        Buffer[Index++] = client.read(); 
      } 
      //Send Data to connected client
      if (strstr(Buffer,"DHT")){
        bzero(Buffer,40);
        float h = dht.readHumidity(); 
        float t = dht.readTemperature(); 
        char str[80];
       
        sprintf(str, "%f,%f", t*9.0 / 5.0 + 32.0,h);
        
        Serial.println(str);
        client.print(str);  
      } 
      else  if (strstr(Buffer,"TST")){
        bzero(Buffer,40);
        int x = connect2Raspberry("tst");
        char str[80];
        client.write(str); 
      }
       
    }
    client.stop();
    Index =0;
    Serial.println("Client disconnected from Server");    
  }
}
