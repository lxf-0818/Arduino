void readADC(char *str) {
        //The voltage read from the battery is around 12v we need to use a voltage divivde to reduce voltage < 3.3V.
        float voltageDivider[5];
        float volt0Alarm =0,volt1Alarm =0,volt2Alarm =0,volt3Alarm =0;
        int j=0;
        char *token = strtok(Buffer+4,"_"); //ptr pass the ADC token to get the actual offset 
        while (token !=NULL) {
          voltageDivider[j++] = atof(token);
          token = strtok(NULL,"_");
        }
        if (!voltageDivider[0]){
          voltageDivider[0] = 5.63; 
          Serial.println("bug stikes again");
        }
        
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
        if (sensorLocation.equalsIgnoreCase("RV")) {
       
          // Prepare your HTTP POST request data
        
          String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
                        + "&location=" + sensorLocation + "&value1=" + String(volts0*voltageDivider[0])
                        + "&value2=" + String(volts1) + "&value3=" + String(volts2) + "";
 
          Serial.print("httpRequestData: ");
          Serial.println(httpRequestData);
          // Send HTTP POST request
          int httpResponseCode = http.POST(httpRequestData);
          if (httpResponseCode>0) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
          }
          else {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
          }
          // Free resources
          http.end();         
        }
}
