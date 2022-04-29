/*
 * Create a TCP Server on ESP8266 NodeMCU. 
 * TCP Socket Server Send Receive
*/
#include <Wire.h>
#define _HOME
//#define CELL
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);
#include <ESP8266WiFi.h>
#include <Adafruit_ADS1X15.h>
#include "Adafruit_MCP9808.h"
#include "DHT.h"
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE); // Initia-lize DHT sensor

Adafruit_ADS1115 adc; 
Adafruit_MCP9808 mcp;

#define PORT 8888 
WiFiServer server(PORT);
WiFiClient client;

#define LOW_VOLT_WARNING 12.7
#define LOW_VOLT_ALARM 12.5

const char* host = "10.0.0.208";

byte LCD_CONFIG,MCP_CONFIG,ADC_CONFIG;
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
//IPAddress local_IP(10,0,0,109);
//IPAddress gateway(10,0,0,1);
//IPAddress subnet(255,255,255,0);
//IPAddress primaryDNS(75,75,75,75);
//IPAddress secondaryDNS(75,75,75,76);
int connect2Raspberry(char*msg);



void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); //Connect to wifi
// if (!WiFi.config(local_IP,gateway,subnet,primaryDNS,secondaryDNS))
//   Serial.println("failed to config");

  // Wait for connection  
  Serial.println("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
    delay(500);
  }
  //used for debug
  //pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(D5, OUTPUT);
  digitalWrite(D5, LOW);  

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  
 
  Serial.print("Port ");
  Serial.println(PORT);  // 
 
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
  char Buf[50];
  String IP = WiFi.localIP().toString();
  IP.toCharArray(Buf+4, 50-4);
  Wire.begin(4,5);
  Wire.beginTransmission(0x48);
  ADC_CONFIG = Wire.endTransmission();
  if (!ADC_CONFIG){
    Serial.println("ADC detected ");
    strncpy(Buf,"ADC:",4);
    connect2Raspberry(Buf);
   
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
    strncpy(Buf,"DHT:",3);
    connect2Raspberry(Buf);
    Serial.println("MCP9808 found");
    // Create the MCP temperature sensor object
    mcp = Adafruit_MCP9808();
    mcp.begin(0x18);
    // Init DHT
   dht.begin();
  }
  

 
  
  server.begin();
  
  
}
void loop() {
  float volt0Alarm =0,volt1Alarm =0,volt2Alarm =0,volt3Alarm =0;
  char str[80];
  float voltageDivider[5];
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
      
      if (strstr(Buffer,"ADC")){
        
        //The voltage read from the battery is around 12v we need to use a voltage divivde to reduce voltage < 3.3V.
        int j=0;
        char *token = strtok(Buffer+4,"_"); //ptr pass the ADC token to get the actual offset 
        while (token !=NULL) {
      //    Serial.println(token);
          voltageDivider[j++] = atof(token);
          token = strtok(NULL,"_");
        }
        bzero(Buffer,40);
        float volts0= adc.computeVolts(adc.readADC_SingleEnded(0));
        if (volts0*voltageDivider[0] < LOW_VOLT_ALARM){
          //connect2Raspberry("LOW_VOLT_ALARM");
          Serial.println("LOW_VOLT_ALARM");
          Serial.printf("v:%f mult:%f\n",volts0,voltageDivider[0]);  
          digitalWrite(D5, HIGH);
          volt0Alarm =1;
        }
        else 
          volt0Alarm =0;
           
        float volts1 = adc.computeVolts(adc.readADC_SingleEnded(1));
        float volts2 = adc.computeVolts(adc.readADC_SingleEnded(2));
        float volts3 = adc.computeVolts(adc.readADC_SingleEnded(3));
        bzero(str,80); 
       
        if (!LCD_CONFIG) {
          lcd.setCursor(0,2);
          lcd.print(volts0);
          lcd.setCursor(0,3);
          lcd.print(volts0*voltageDivider[0]);
        } 
        sprintf(str,"%f,%f,%f,%f,%f",volts0*voltageDivider[0],volts1,volts2,volts3,volt0Alarm); 
        break;
      }
 
      else if (strstr(Buffer,"CLR")){
        bzero(Buffer,40);
        digitalWrite(D5, LOW);
        strcpy(str,"ALARM RESET");
        break;
      }
      else if (strstr(Buffer,"DHT")){
        bzero(Buffer,40);
        float f=0.0;
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

      
      else {
        strcpy(str,"Invalid cmd");
        break;
      }
    }
    Serial.println(str);
    client.print(str); 
    client.stop();
    Serial.println("Client disconnected from Server");    
  }
}
