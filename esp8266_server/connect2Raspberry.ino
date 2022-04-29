int connect2Raspberry(char*msg) {
    WiFiClient client;
    if (!client.connect(host, 1233)) {
      Serial.println("connection failed");
      delay(5000);
      return 0 ;
    }
    
  
   // This will send a string to the server
   if (client.connected()) {
     Serial.printf("msg sent to Pi %s\n",msg);
     client.printf(msg);
   }
   String line = client.readStringUntil('\n');
   Serial.println(line);

   Serial.println("closing connection");
   client.stop();
   return 0;
    
}
