#include <SdFat.h>
#include <SdFatUtil.h>  // define FreeRam()
#include <Wire.h>
#include "RTClib.h"

// Serial print stream
ArduinoOutStream cout(Serial);

RTC_DS1307 RTC; // define the Real Time Clock object
//------------------------------------------------------------------------------
// call back for file timestamps
void dateTime(uint16_t* date, uint16_t* time) {
    DateTime now = RTC.now();

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}
//------------------------------------------------------------------------------
// format date/time
ostream& operator << (ostream& os, DateTime& dt) {
  os << dt.year() << '/' << int(dt.month()) << '/' << int(dt.day()) << ',';
  os << int(dt.hour()) << ':' << setfill('0') << setw(2) << int(dt.minute());
  os << ':' << setw(2) << int(dt.second()) << setfill(' ');
  return os;
}

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

// the logging file
ofstream logfile;

//New variable for the SD card
SdFat sd;

// buffer to format data - makes it eaiser to echo to Serial
char buf[80];

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
 
  // red LED indicates error
  //digitalWrite(redLEDpin, HIGH);

  while(1);
}

void setup(void) {
  Serial.begin(9600);
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
 
    // pstr stores strings in flash to save RAM
  cout << endl << pstr("FreeRam: ") << FreeRam() << endl;
 
    // format header in buffer
  obufstream bout(buf, sizeof(buf));

  bout << pstr("millis");

  // connect to RTC
  Wire.begin(); 
  if (!RTC.begin()) {
    bout << pstr("RTC failed");
    Serial.println("RTC failed");
  }

  SdFile::dateTimeCallback(dateTime);
  DateTime now = RTC.now();
  cout  << now << endl;
 
  if (!sd.init(SPI_HALF_SPEED, chipSelect)) sd.initErrorHalt(); 
 
  //Get the current time from the Real Time Clock (RTC);
  now = RTC.now();   
 
  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (sd.exists(filename)) continue;
    logfile.open(filename);
    break;   
  }
 
  if (!logfile.is_open()) {
    Serial.println("Could not create the file!");
    error("couldnt create file");
  }
 
  Serial.print("Logging to: ");
  Serial.println(filename);

  // If you want to set the aref to something other than 5v
  analogReference(EXTERNAL);
}

void loop(void) {

}
