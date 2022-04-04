

/*
 * Create a TCP Server on ESP8266 NodeMCU. 
 * TCP Socket Server Send Receive
*/
#define _HOME
//#define CELL
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);

#include <ESP8266WiFi.h>
int port = 8888;  
const char* host = "10.0.0.208";
int x =0;
WiFiServer server(port);

//Server connect to WiFi Network
#ifdef _HOME
const char *ssid = "NETGEAR37-2"; 
const char *password = "grandcurtain880";
#endif
#ifdef CELL
char *ssid = "Verizon-MiFi6620L-E497"; 
const char *password = "4458e951";
#endif  
int Index=0;
char Buffer[40];
byte LCD_CONFIG;
//IPAddress local_IP(10,0,0,109);
//IPAddress gateway(10,0,0,1);
//IPAddress subnet(255,255,255,0);
//IPAddress primaryDNS(75,75,75,75);
//IPAddress secondaryDNS(75,75,75,76);

#include "DHT.h"
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor

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
     client.printf("DHT:%s",msg);
   }
   String line = client.readStringUntil('\n');
   Serial.println(line);

   Serial.println("closing connection");
   client.stop();
   return 0;
    
}

void setup() {
  Wire.begin(4,5);
  Wire.beginTransmission(39);
  LCD_CONFIG = Wire.endTransmission();
  if (!LCD_CONFIG){
    lcd.init();                      // initialize the lcd 
    lcd.backlight();
  }
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); //Connect to wifi
// if (!WiFi.config(local_IP,gateway,subnet,primaryDNS,secondaryDNS))
//   Serial.println("failed to config");

  //used for debug
  //pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  //pinMode(D5, OUTPUT);
  //digitalWrite(D5, LOW);
 
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
  
  String IP = WiFi.localIP().toString();
  char Buf[50];
  IP.toCharArray(Buf, 50);
  connect2Raspberry(Buf);
  server.begin();
  Serial.print(WiFi.localIP());
  if (!LCD_CONFIG)
    lcd.print(WiFi.localIP());
  Serial.print(" on port ");
  Serial.println(port);
 
  

  // Init DHT
  dht.begin();
}
void loop() {
  char str[80];
  WiFiClient client = server.available();
  if (client) {
    if(client.connected()) {
      Serial.print(client.remoteIP());
      Serial.println("  Raspberry(Client) Connected to Server");
    }
      
    while(client.connected()){      
      while(client.available()>0){
        // read data from the connected client
        Buffer[Index++] = client.read(); 
      } 
      
      if (strstr(Buffer,"DHT")){
        
        bzero(Buffer,40);
        float h = dht.readHumidity(); 
        float t = dht.readTemperature();
        if (!LCD_CONFIG) {
          lcd.setCursor(0,1);
          lcd.print(t*9.0 / 5.0 + 32.0);
          lcd.setCursor(0,2);
          lcd.print(h);
        }   
        sprintf(str, "%f,%f", t*9.0 / 5.0 + 32.0,h);
        Serial.println(str);
        client.print(str);  //Send Data to connected client
      } 
      if (strstr(Buffer,"CLR")){
        bzero(Buffer,40);
        strcpy(str,"alarm cleared");
        digitalWrite(D5, LOW);
        client.print(str); 
      }

    }
    client.stop();
    Index =0;
    Serial.println("Client disconnected from Server");    
  }
}
