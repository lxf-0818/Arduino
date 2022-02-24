/*
 * Create a TCP Server on ESP8266 NodeMCU. 
 * TCP Socket Server Send Receive
*/
#include <ESP8266WiFi.h>
int port = 8888;  //Port number
WiFiServer server(port);
//Server connect to WiFi Network
const char *ssid = "NETGEAR37-2";  //Enter your wifi SSID
const char *password = "grandcurtain880";  //Enter your wifi Password
int Index=0;
char Buffer[40];
#include "DHT.h"
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor

void setup() 
{
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
      Serial.println("Client Connected");
      
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
        client.write(str);  
      } 
      else  if (strstr(Buffer,"TST")){
        bzero(Buffer,40);
        for (int i =0 ; i<1 ;i++) {
          digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (negative logic) 
          delay(1000);                      // Wait for a second
          digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
          delay(2000);                      // Wait for two seconds (to demonstrate the active low LED) 
        }
       char str[80];
       client.write(str); 
      }
       
    }
    client.stop();
    Index =0;
    Serial.println("Client disconnected");    
  }
}
