

/*
 * Create a TCP Server on ESP8266 NodeMCU. 
 * TCP Socket Server Send Receive
*/
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_MCP9808.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>

#define _HOME
//#define CELL
#ifdef _HOME
const char* host = "10.0.0.208";
const char *ssid = "NETGEAR37-2"; 
const char *password = "grandcurtain880";
#endif
#ifdef CELL
char *ssid = "Verizon-MiFi6620L-E497"; 
const char *password = "4458e951";
const char* host = "192.168.1.2";
#endif  

#define LOW_VOLT_WARNING 12.7
#define LOW_VOLT_ALARM 12.5
#define SEALEVELPRESSURE_HPA (1013.25)
#define PORT 8888 

LiquidCrystal_I2C lcd(0x27,20,4);
Adafruit_BME280 bme; // I2C
Adafruit_ADS1115 adc; 
Adafruit_MCP9808 mcp;

// GPIO where the DS18B20 is connected to
const int oneWireBus = 0; 
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
//Server connect to WiFi Network
WiFiServer server(PORT);
WiFiClient client;

byte LCD_CONFIG=1,MCP_CONFIG=1,ADC_CONFIG=1,DS0_CONFIG=1,BME_CONFIG=1;
int Index=0;
char Buffer[40];
byte addr[8];  
int connect2Raspberry(char*msg);
int readTemp(char * str);
void readADC(char *str);
 

void setup() {
  char Buf[50];
  Serial.begin(115200);
  pinMode(D5, OUTPUT);
  digitalWrite(D5, LOW);  

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); //Connect to wifi
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
  Serial.print("Port ");
  Serial.println(PORT);  // 
 
  //The following devices use I2C if found sets *_CONFIG =0 else set to 1
  Wire.begin(4,5);
  Wire.beginTransmission(0x27);
  LCD_CONFIG = Wire.endTransmission();
  if (!LCD_CONFIG){
    lcd.init();                      // initialize the lcd 
    lcd.backlight();
    lcd.print(WiFi.localIP());
    lcd.setCursor(0,1);
    lcd.print(PORT);
  }
 
  Wire.begin(4,5);
  Wire.beginTransmission(0x48);
  ADC_CONFIG = Wire.endTransmission();
  if (!ADC_CONFIG){
    Serial.println("ADC detected ");
    strncpy(Buf,"ADC:",4);
    adc.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
    if (!adc.begin()) {
      Serial.println("Failed to initialize ADS.");
      while (1);
    }
  }
  
  Wire.begin(4,5);
  Wire.beginTransmission(0x18);
  MCP_CONFIG = Wire.endTransmission();
  if (!MCP_CONFIG){
    strncpy(Buf,"MCP:",4);
    Serial.println("MCP9808 detected");
    // Create the MCP temperature sensor object
    mcp = Adafruit_MCP9808();
    mcp.begin(0x18);
  }
  Wire.begin(4,5);
  Wire.beginTransmission(0x77);
  BME_CONFIG = Wire.endTransmission();
  if (!BME_CONFIG){
    Serial.println("BME280 detected ");
    strncpy(Buf,"BME:",4);
    if (!bme.begin(0x77, &Wire)) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      BME_CONFIG=1;
   }
  }

  //Following same logic as I2C if oneWire detects device DS0_CONFIG =0 else 1 
  int deviceCount =0;
  while (1){
    if (!oneWire.search(addr)) {
      if (deviceCount) { 
        strncpy(Buf,"DS0:",4);
        Serial.printf("DS18B20 detected cnt:%d\n",deviceCount);
        oneWire.reset_search();
        DS0_CONFIG =0;
      }
      else 
        DS0_CONFIG =1;

      break;
    }
    deviceCount++;
    
  }

  if (DS0_CONFIG && MCP_CONFIG && ADC_CONFIG && BME_CONFIG)
    Serial.println("no devices detected");
  else {
    String IP = WiFi.localIP().toString();
    IP.toCharArray(Buf+4, 50-4);
    connect2Raspberry(Buf);
  }

  server.begin();
 
}
void loop() {
  
  char str[80];

  bool deviceNotEnabled =false;
  client = server.available();
  if (client) {
    if(client.connected()) { 
      Serial.print(client.remoteIP());
      Serial.println("  Client(Raspberry) Connected to Server");
    }
    while(1){      
      while(client.available()>0){
        // read data from the connected client
        Buffer[Index++] = client.read(); 
      } 
      if (!Index) {
        continue; // Wait till client sends command
      }
      Index =0;// reset length of command for next i/o
      
      if (strstr(Buffer,"RST")){
        client.stop();
        Serial.println("Client disconnected from Server+rst");    
        delay(1000);
        ESP.restart();  
      }
      if (strstr(Buffer,"ADC")){
        if (ADC_CONFIG){
          deviceNotEnabled = true;
          break;
        }
        readADC(str);
        break;
      }
      else if (strstr(Buffer,"CLR")){ 
        digitalWrite(D5, LOW);
        strcpy(str,"ALARM RESET");
        break;
      }
      else if (strstr(Buffer,"MCP")){
        if (!MCP_CONFIG){
          mcp.wake();
          float f = mcp.readTempF();
          mcp.shutdown();
          sprintf(str,"%f",f);
          break;
        }
        else {
          deviceNotEnabled = true;
          break;
        }
     }
     else if (strstr(Buffer,"DS0")){
        if(!DS0_CONFIG){
          readTemp(str);
          break;
         }
         else {
          deviceNotEnabled = true;
          break;
        }
     }
    else if (strstr(Buffer,"BME")){
        if(!BME_CONFIG){
          sprintf(str,"%f_%f_%f_%f",bme.readTemperature()*1.8+32,bme.readPressure() / 100.0F,bme.readAltitude(SEALEVELPRESSURE_HPA),bme.readHumidity());
          break;
         }
         else {
          deviceNotEnabled = true;
          break;
        }
     }
  
     else {
        strcpy(str,"Invalid cmd");
        break;
      }
    }
    if (deviceNotEnabled) {
       strcpy(str,"0");
     //  Serial.printf("No devive found for %s on %s\n",Buffer,Buf+4);  
    }
    Serial.println(str);
    client.print(str); 
    client.stop();
    Serial.println("Client disconnected from Server");    
  }
}
