/*
 * Create a TCP Server on ESP8266 NodeMCU. 
 * TCP Socket Server Send Receive
*/
//#define _DHT
#define _ADS
//#include <Wire.h>
#include <ESP8266WiFi.h>

#ifdef _ADS
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads; 
#define LOW_VOLT_WARNING 2.3
#define LOW_VOLT_ALARM 2.1

#endif 

int port = 8888;  
const char* host = "10.0.0.208";

WiFiServer server(port);
//Server connect to WiFi Network
const char *ssid = "NETGEAR37-2"; 
const char *password = "grandcurtain880";  
int Index=0;
char Buffer[40];

#ifdef _DHT
#include "DHT.h"
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor
#endif


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
  
#ifdef _DHT
  // Init DHT
  dht.begin();
#endif  
#ifdef _ADS
  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }

#endif
}
void loop() {

  WiFiClient client = server.available();
  if (client) {
    if(client.connected())  
      Serial.print(client.remoteIP());
      Serial.println("  Raspberry(Client) Connected to Server");
      
    while(client.connected()){      
      while(client.available()>0){
        // read data from the connected client
        Buffer[Index++] = client.read(); 
      } 
      //Send Data to connected client
#ifdef _DHT
      if (strstr(Buffer,"DHT")){
        bzero(Buffer,40);
        float h,t;
        for (int i =0;i<4;i++){
          h = dht.readHumidity(); 
          t = dht.readTemperature();
         // delay(500);
        }           
        char str[80];
       
        sprintf(str, "%f,%f", t*9.0 / 5.0 + 32.0,h);
        
        Serial.println(str);
        client.print(str);  
      } 
#endif
#ifdef _ADS
     
      float volts0, volts1, volts2, volts3;
      if (strstr(Buffer,"MB")){
        bzero(Buffer,40);
        volts0 = ads.computeVolts(ads.readADC_SingleEnded(0));
        if (volts0 < LOW_VOLT_WARNING)
          connect2Raspberry("LOW_VOLT_WARNING");
        
            
        volts1 = ads.computeVolts(ads.readADC_SingleEnded(1));
        volts2 = ads.computeVolts(ads.readADC_SingleEnded(2));
        volts3 = ads.computeVolts(ads.readADC_SingleEnded(3));
      
        char str[80];
        bzero(str,80); 
        sprintf(str,"%f,%f,%f,%f",volts0,volts1,volts2,volts3);  
        Serial.println(str);    
        client.print(str);  
      } 

#endif
    }
    client.stop();
    Index =0;
    Serial.println("Client disconnected from Server");    
  }
}
