/*
  SD card datalogger
  by Tom Igoe
  hacked by Peter Marks to write time stamps

 This example code is in the public domain.
 */


#include <SD.h>

// On the Ethernet Shield, CS is pin 4. Note that even if it's not
// used as the CS pin, the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.
const int chipSelect = 4;

/*
// This is to talk to the real time clock
#include "Wire.h"
#define DS1307_I2C_ADDRESS 0x68  // This is the I2C address
*/

// Global Variables
int i;
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;


void setup()
{
  /*
  Wire.begin();
  */
  Serial.begin(9600);
  Serial.print("Initializing SD card...\n");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(chipSelect, OUTPUT);
  Serial.print("chipSelect set to output\n");
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) 
  {
    Serial.println("Card failed, or not present\n");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.\n");
}


void loop()
{
  /*
  // make a string for assembling the data to log:
  String dataString = getDateDs1307();
  dataString += String(",");
  */
  
  // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 3; analogPin++) 
  {
    int sensor = analogRead(analogPin);
   /*
   dataString += String(sensor);
   */
    if (analogPin < 2) 
 /*
 {
      dataString += ","; 
    }
    */
  }


  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.csv", FILE_WRITE);


  // if the file is available, write to it:
  if (dataFile) 
  {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }  
  // if the file isn't open, pop up an error:
  else 
  {
    Serial.println("error opening datalog.csv");
  } 
  delay(1000 * 10);
}



/*
// Gets the date and time from the ds1307 and return
// result in a format a spreadsheet can parse: 06/10/11 15:10:00
String getDateDs1307()
{
  // Reset the register pointer
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.send(0x00);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_I2C_ADDRESS, 7);

  // A few of these need masks because certain bits are control bits
  second     = bcdToDec(Wire.receive() & 0x7F);
  minute     = bcdToDec(Wire.receive());
  hour       = bcdToDec(Wire.receive() & 0x3F);  
  dayOfWeek  = bcdToDec(Wire.receive());
  dayOfMonth = bcdToDec(Wire.receive());
  month      = bcdToDec(Wire.receive());
  year       = bcdToDec(Wire.receive());
  
  String dataString = "";
  
  dataString += Print2Digit(dayOfMonth); 
  dataString += String("/");
  dataString += Print2Digit(bcdToDec(month)); 
  dataString += String("/"); // Y2k1 bug!
  dataString += Print2Digit(bcdToDec(year));
  dataString += String(" ");
  dataString += Print2Digit(hour);
  dataString += String(":");
  dataString += Print2Digit(minute);
  dataString += String(":");
  dataString += Print2Digit(second);


  return dataString;
}
*/

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
}

String Print2Digit(byte Val)
{
  String dataString = "";
  if (Val < 10)
  {
    dataString = "0";
  }  
  dataString += String(Val, DEC);
  return dataString;
}
