int readTemp(char * str) { 
  byte data[12],present = 0;
  float temp[10];
  int deviceCount =0;
  
  while (1){
    
     if (!oneWire.search(addr)) {
     // Serial.println("No more addresses.");
      oneWire.reset_search();
      if (!deviceCount) strcpy(str,"devices where dropped check wiring");
      break;
    }
    if (OneWire::crc8(addr, 7) != addr[7]) {
      strcpy(str,"CRC ROM is not valid!");
      Serial.print("ROM =");
      for(int i = 0; i < 8; i++) {
        Serial.write(' ');
        Serial.print(addr[i], HEX);
      }
      
      return 1;
    }


    oneWire.reset();
    oneWire.select(addr);
    oneWire.write(0x44, 1);        // start conversion, with parasite power on at the end
    delay(1000);                   // maybe 750ms is enough?
    present = oneWire.reset();
    oneWire.select(addr);    
    oneWire.write(0xBE);           // Read Scratchpad
    for ( int i = 0; i < 9; i++)   // we need 9 bytes
      data[i] = oneWire.read();
  
    if (OneWire::crc8(data, 8) ==data[8]){
      int16_t raw = (data[1] << 8) | data[0];
      temp[deviceCount++] = (float)raw / 16.0 * 1.8 + 32.0;
    }
    else {
      strcpy(str,"CRC RAW is not valid!");
      Serial.print("  Data = ");
      Serial.print(present, HEX);
      Serial.print(" ");
      for (int i = 0; i < 9; i++) {           // we need 9 bytes
        data[i] = oneWire.read();
        Serial.print(data[i], HEX);
        Serial.print(" ");
      }
      return 2;
    }
  }
  int offset = 0;
  for(int i =0 ;i<deviceCount;i++) {
    sprintf(str+offset,"%f,",temp[i]);
    offset += strlen(str);
 
  }
  if (deviceCount) 
    str[(strlen(str)-1)] ='0'; // remove last ','
  else {
    strcpy(str,"device(s) where dropped check wiring");
    return 3 ;
  }
   return 0;
 
}
