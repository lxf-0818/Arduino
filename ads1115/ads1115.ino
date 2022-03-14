#include <Wire.h>
#include <Adafruit_ADS1X15.h>
// Using default gain 
//ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)

Adafruit_ADS1X15 ads;
float Voltage = 0.0;

void setup(void) 
{
Serial.begin(115200); 
ads.begin();
}

void loop(void) 
{
int16_t adc0;

adc0 = ads.readADC_SingleEnded(0);
float volts0 = ads.computeVolts(adc0);
Voltage = (adc0 * 0.1875)/1000;

Serial.print("AIN0: "); 
Serial.print(adc0);
Serial.print("\tVoltage: ");
Serial.print(Voltage, 7);
Serial.print("\tVoltage func, "); 
Serial.println(volts0,7); 
Serial.println();

delay(1000);
}
