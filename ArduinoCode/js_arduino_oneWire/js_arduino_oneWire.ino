
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into pin 2 on the Arduino (can be any digital I/O pin)
#define ONE_WIRE_BUS 13 // oneWire bus bin

OneWire oneWire(ONE_WIRE_BUS); // setup oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire); // pass our oneWire reference to Dallas Temperature
float tempValue[] = {
  0, 1};

int numberOfSensors; // will store how many sensors we have
byte i; // will hold value for different loops

//////////////////////////////////// setup ////////////////////////////////////
void setup(void)
{
  // start serial port
  Serial.begin(9600);
  Serial.println("Started serial communication");
  Serial.println("Starting 1-wire sensors...");
  sensors.begin(); // start up the oneWire library
  delay(5000);  //important on linux as serial port can lock up otherwise
  Serial.println("Discovering and counting sensors...");
  numberOfSensors = discoverOneWireDevices(); // count sensors
  Serial.println();
}

//////////////////////////////////// main loop ////////////////////////////////////
void loop(void)
{
  getTemperatures();
  delay(10000); //wait 10 sec
}

/////////////////////////////// request temperatures ///////////////////////////////
void getTemperatures(void) {
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  for(i=0; i < numberOfSensors; i++) { // read each of our sensors and print the value
    Serial.print("Temperature for Device ");
    Serial.print( i );
    Serial.print(" is: ");
    //Serial.println( sensors.getTempCByIndex(i) ); // 0 refers to the first IC on the wire
    tempValue[i] = sensors.getTempCByIndex(i);
    Serial.println(tempValue[i]);
  }
  Serial.println();
}

/////////////////////////////// discover sensors ////////////////////////////////
int discoverOneWireDevices(void) {
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

