/*
 * Create a TCP Server on ESP8266 NodeMCU. 
 * TCP Socket Server Send Receive
*/
#define _HOME
//#define CELL


//#include <Wire.h>
#include <ESP8266WiFi.h>
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads; 
#define LOW_VOLT_WARNING 2.3
#define LOW_VOLT_ALARM 2.22
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
//IPAddress local_IP(10,0,0,109);
//IPAddress gateway(10,0,0,1);
//IPAddress subnet(255,255,255,0);
//IPAddress primaryDNS(75,75,75,75);
//IPAddress secondaryDNS(75,75,75,76);

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
// if (!WiFi.config(local_IP,gateway,subnet,primaryDNS,secondaryDNS))
//   Serial.println("failed to config");

  //used for debug
  //pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(D5, OUTPUT);
  digitalWrite(D5, LOW);
 
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
  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
}
void loop() {
  char str[80];
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
      float volts0, volts1, volts2, volts3;
      if (strstr(Buffer,"ADC")){
        bzero(Buffer,40);
        volts0= ads.computeVolts(ads.readADC_SingleEnded(0));
        if (volts0 < LOW_VOLT_ALARM){
          //connect2Raspberry("LOW_VOLT_ALARM");
          Serial.println("LOW_VOLT_ALARM");
          digitalWrite(D5, HIGH);
  
        }
         
        volts1 = ads.computeVolts(ads.readADC_SingleEnded(1));
        volts2 = ads.computeVolts(ads.readADC_SingleEnded(2));
        volts3 = ads.computeVolts(ads.readADC_SingleEnded(3));
      
        bzero(str,80); 
        sprintf(str,"%f,%f,%f,%f",volts0,volts1,volts2,volts3);  
        Serial.println(str);    
        client.print(str);  
      } 
      if (strstr(Buffer,"CLR")){
        bzero(Buffer,40);
        digitalWrite(D5, LOW);
        strcpy(str,"ALARM RESET");
        client.print(str); 
      }
    }
    client.stop();
    Index =0;
    Serial.println("Client disconnected from Server");    
  }
}
