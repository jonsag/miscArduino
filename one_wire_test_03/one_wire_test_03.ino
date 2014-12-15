// OneWire and DallasTemperature libraries from
//   http://milesburton.com/index.php?title=Dallas_Temperature_Control_Library
// Code based on examples from above and at
//   http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html
// See also http://www.arduino.cc/playground/Learning/OneWire
 
#include <OneWire.h>
#include <DallasTemperature.h>
 
// Data wire is plugged into pin 2 on the Arduino (can be any digital I/O pin)
#define ONE_WIRE_BUS 13
 
// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
//DallasTemperature sensors(&amp;oneWire);
DallasTemperature sensors(&oneWire);
 
int numberOfSensors;
 
void setup(void)
{
  // start serial port
  Serial.begin(9600);
  Serial.println("1-Wire DS18B20 example code");
 
  // Start up the library
  sensors.begin();
 
  delay(5000);  //important on linux as serial port can lock up otherwise
  numberOfSensors = discoverOneWireDevices();
  Serial.println();
}
 
void loop(void)
{
  printTemperaturesToSerial();
  delay(10000); //wait 10 sec
}
 
void printTemperaturesToSerial(void) {
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
 
  // Read each of our sensors and print the value
  for(int i=0; i < numberOfSensors; i++) {
   Serial.print("Temperature for Device ");
   Serial.print( i );
   Serial.print(" is: ");
   // Why "byIndex"? You can have more than one IC on the same bus. 
   // 0 refers to the first IC on the wire
   Serial.println( sensors.getTempCByIndex(i) );
  }
 
  Serial.println();
}
 
// Based on http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html
int discoverOneWireDevices(void) {
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  int count = 0;
 
  Serial.println("Looking for 1-Wire devices...");
  while(oneWire.search(addr)) {
    Serial.print("Found \'1-Wire\' device with address: ");
    for( i = 0; i < 8; i++) {
      Serial.print("0x");
      if (addr[i] < 16) {
        Serial.print('0');
      }
      Serial.print(addr[i], HEX);
      if (i < 7) {
        Serial.print(", ");
      }
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
        Serial.println("CRC is not valid!");
        return 0;
    }
    Serial.println();
    count++;
  }
  Serial.println("That's it.");
  oneWire.reset_search();
  return count;
}
