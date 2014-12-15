// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
 
#include <Wire.h>
#include "RTClib.h"
 
RTC_DS1307 RTC;

int squareWave = 2;

int greenLED = 13;
 
void setup () {
    Serial.begin(9600);
    Wire.begin();
    RTC.begin();
 
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  
  pinMode(greenLED, OUTPUT);
  pinMode(squareWave, INPUT);
 
}
 
void loop () {
  
  digitalWrite(greenLED, HIGH);
  
    DateTime now = RTC.now();
 
    Serial.print(now.year(), DEC);
    Serial.print("-");
        if (now.month() < 10) {
      Serial.print("0");
    }
    Serial.print(now.month(), DEC);
    Serial.print("-");
        if (now.day() < 10) {
      Serial.print("0");
    }
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(":");
    if (now.minute() < 10) {
      Serial.print("0");
    }
    Serial.print(now.minute(), DEC);
    Serial.print(":");
        if (now.second() < 10) {
      Serial.print("0");
    }
    Serial.print(now.second(), DEC);
    Serial.println();
 
    Serial.print(" since 1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");
 
    // calculate a date which is 7 days and 30 seconds into the future
    DateTime future (now.unixtime() + 7 * 86400L + 30);
 
    Serial.print(" now + 7d + 30s: ");
    Serial.print(future.year(), DEC);
    Serial.print('/');
    Serial.print(future.month(), DEC);
    Serial.print('/');
    Serial.print(future.day(), DEC);
    Serial.print(' ');
    Serial.print(future.hour(), DEC);
    Serial.print(':');
        if (future.minute() < 10) {
      Serial.print("0");
    }
    Serial.print(future.minute(), DEC);
    Serial.print(':');
        if (future.second() < 10) {
      Serial.print("0");
    }
    Serial.print(future.second(), DEC);
    Serial.println();
 
    Serial.println();
    delay(3000);
}
