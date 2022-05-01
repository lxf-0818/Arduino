

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
byte LCD_CONFIG,MCP_CONFIG;
//IPAddress local_IP(10,0,0,109);
//IPAddress gateway(10,0,0,1);
//IPAddress subnet(255,255,255,0);
//IPAddress primaryDNS(75,75,75,75);
//IPAddress secondaryDNS(75,75,75,76);
#include "Adafruit_MCP9808.h"
int connect2Raspberry(char*msg);
 Adafruit_MCP9808 mcp;

#include "DHT.h"
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor


void setup() {
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); //Connect to wifi
// if (!WiFi.config(local_IP,gateway,subnet,primaryDNS,secondaryDNS))
//   Serial.println("failed to config");

  //used for debug
  //pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output

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
  
 
  Wire.begin(4,5);
  Wire.beginTransmission(0x27);
  LCD_CONFIG = Wire.endTransmission();
  if (!LCD_CONFIG){
    lcd.init();                      // initialize the lcd 
    lcd.backlight();
  }
   char Buf[50];
  Wire.begin(4,5);
  Wire.beginTransmission(0x18);
  MCP_CONFIG = Wire.endTransmission();
  if (!MCP_CONFIG){
    strcpy(Buf,"DHT:");
    Serial.println("MCP9808 found");
    // Create the MCP temperature sensor object
    mcp = Adafruit_MCP9808();
    mcp.begin(0x18);
  }
  

  Serial.print(WiFi.localIP());
  if (!LCD_CONFIG)
    lcd.print(WiFi.localIP());
    lcd.setCursor(0,1);
    lcd.print(port);
    
  Serial.print(" on port ");
  Serial.println(port);

  String IP = WiFi.localIP().toString();
  
  IP.toCharArray(Buf+4, 50);
  connect2Raspberry(Buf);
  server.begin();
  
    
 
  // Init DHT
  dht.begin();
}
void loop() {
  char str[80];
  float f=0.0;
  WiFiClient client = server.available();
  if (client) {
    if(client.connected()) {
      Serial.print(client.remoteIP());
      Serial.println("  Client(Rapsberry) Connected to Server");
    }
      
    while(1){      
      while(client.available()>0){
        // read data from the connected client
        Buffer[Index++] = client.read(); 
      } 
      if (!Index) {
       // delay(500);
        continue; // Wait till client sends command
      }
      Index=0;
      
      if (strstr(Buffer,"DHT")){
        bzero(Buffer,40);
        float h = dht.readHumidity(); 
        float t = dht.readTemperature();
        if (!LCD_CONFIG) {
          lcd.setCursor(0,2);
          lcd.print(t*9.0 / 5.0 + 32.0);
          lcd.setCursor(0,3);
          lcd.print(h);
        } 
        if (!MCP_CONFIG){
          mcp.wake();
          f = mcp.readTempF();
          Serial.print("Temp: "); 
          Serial.print(f, 4); 
          Serial.println("*F.");
          mcp.shutdown();
        }
 
  
    
        sprintf(str, "%f,%f", t*9.0 / 5.0 + 32.0,f);
        break;
      } 

      else 
        strcpy(str,"Invalid cmd");
        break;
    }
    Serial.println(str);
    client.print(str); 
    client.stop();
    Serial.println("Client disconnected from Server");    
  }
}
