// Date and time functions using a DS1307 RTC connected via I2C and Wire lib

#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 RTC;

int squareWaveIn = 2;
int greenLED = 13;

int squareWave = 0;

void setup () {
  Serial.begin(9600);
  Serial.println("RTClib_testing_01");
  Wire.begin();
  RTC.begin();

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  
  RTC.writeSqwPinMode(SquareWave1HZ);

  pinMode(greenLED, OUTPUT);
  pinMode(squareWaveIn, INPUT);

}

void loop () {

  squareWave = digitalRead(squareWaveIn);
  
  if (squareWave == HIGH) {  
    digitalWrite(greenLED, HIGH);
  }
  else {
    digitalWrite(greenLED, LOW);
  }

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

  Serial.println();
  delay(3000);
}

