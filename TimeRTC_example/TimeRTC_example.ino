/*
 * TimeRTC.pde
 * example code illustrating Time library with Real Time Clock.
 * 
 */

#include <Time.h>  
#include <Wire.h>  
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t

void setup()  {
  Serial.begin(9600);
  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if(timeStatus()!= timeSet) 
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");      
}

void loop()
{
  digitalClockDisplay();  
  delay(1000);
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(year());
  printDigitsMinus(month());
  printDigitsMinus(day());
  Serial.print(" ");
  Serial.print(hour());
  printDigitsColon(minute());
  printDigitsColon(second());
  Serial.println(); 
}

void printDigitsColon(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void printDigitsMinus(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print("-");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

