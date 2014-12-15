// Date and time functions using a DS1307 RTC connected via I2C and Wire lib

/*
 The circuit:
 *  GND                              display pin 1
 *  +5V supply voltage for logic     display pin 2
 
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO - contrast        display pin 3
 
 * LCD RS pin to digital pin 12      display pin 4
 * LCD R/W pin to ground             display pin 5
 * LCD Enable pin to digital pin 11  display pin 6
 * LCD D4 pin 11 - to digital pin 6  display pin 11
 * LCD D5 pin 12 - to digital pin 5  display pin 12
 * LCD D6 pin 13 - to digital pin 4  display pin 13
 * LCD D7 pin 14 - to digital pin 3  display pin 14
 
 * 10K resistor:
 * ends to +5V and ground
 * wiper to back light anode:+4.2V   display pin 15
 * GND to back light cathode         display pin 16
 */

#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal.h>

RTC_DS1307 RTC;

int squareWaveIn = 2;
int greenLED = 13;

int squareWave = 0;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 6, 5, 4, 3);

double displayedCurrent[] = { // the displayed value on the web page
  1, 2, 3};
double ackDisplayedCurrent[] = { // all reads ackumulated
  1, 2, 3};

const int phaseR1 = A0; // declare the analog inputs
const int phaseS2 = A1;
const int phaseT3 = A2;

byte currentDisplayedCounter = 0;
byte addCurrents = 0;

void setup () {
  Serial.begin(9600);
  Serial.println("RTC_shield_with_LCD");
  Wire.begin();
  RTC.begin();

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    RTC.adjust(DateTime(__DATE__, __TIME__)); // sets the RTC to the date & time this sketch was compiled
  }

  RTC.writeSqwPinMode(SquareWave1HZ);

  pinMode(greenLED, OUTPUT);
  pinMode(squareWaveIn, INPUT);

  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);
  // Print a message to the LCD.
  //lcd.print("Time now");

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

  /*
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
   */

  // year
  lcd.setCursor(0, 0);
  lcd.print(now.year(), DEC);

  lcd.setCursor(4, 0);
  lcd.print("-");

  // month
  lcd.setCursor(5, 0);
  if (now.month() < 10) {
    lcd.print("0");
    lcd.setCursor(6, 0);    
  }
  lcd.print(now.month(), DEC);

  lcd.setCursor(7, 0);
  lcd.print("-");

  // day
  lcd.setCursor(8, 0);
  if (now.day() < 10) {
    lcd.print("0");
    lcd.setCursor(9, 0);    
  }
  lcd.print(now.day(), DEC);

  // hour
  lcd.setCursor(0, 1);
  if (now.hour() < 10) {
    lcd.print("0");
    lcd.setCursor(1, 1);    
  }
  lcd.print(now.hour(), DEC);

  lcd.setCursor(2, 1);
  lcd.print(":");

  // minute
  lcd.setCursor(3, 1);
  if (now.minute() < 10) {
    lcd.print("0");
    lcd.setCursor(4, 1);    
  }
  lcd.print(now.minute(), DEC);

  lcd.setCursor(5, 1);
  lcd.print(":");
  // second
  lcd.setCursor(6, 1);
  if (now.second() < 10) {
    lcd.print("0");
    lcd.setCursor(7, 1);    
  }
  lcd.print(now.second(), DEC);

  readCurrents();

  //delay(100);
}

/////////////////////////////// read currents ///////////////////////////////
void readCurrents(void) {
  //Serial.println("Delaying...");
  if (currentDisplayedCounter < 100) {
    //for(discardCounter=1; discardCounter <= 2; discardCounter++) {
    //  waste = analogRead(phaseR1);
    //}
    ackDisplayedCurrent[1] = ackDisplayedCurrent[1] + abs((analogRead(phaseR1) - 511) * 0.35); // read currents
    //for(discardCounter=1; discardCounter <= 2; discardCounter++) {
    //  waste = analogRead(phaseS2);
    //}
    ackDisplayedCurrent[2] = ackDisplayedCurrent[2] + abs((analogRead(phaseS2) - 511) * 0.35);
    //for(discardCounter=1; discardCounter <= 2; discardCounter++) {
    //  waste = analogRead(phaseT3);
    //}
    ackDisplayedCurrent[3] = ackDisplayedCurrent[3] + abs((analogRead(phaseT3) - 511) * 0.35);
    currentDisplayedCounter++;
  }
  else {
    for(addCurrents=1; addCurrents <= 3; addCurrents++) {
      displayedCurrent[addCurrents] = ackDisplayedCurrent[addCurrents] / 100; // set the displayed current value
      lcd.setCursor(addCurrents * 7 - 7, 2);
      lcd.print(displayedCurrent[addCurrents]);
      ackDisplayedCurrent[addCurrents] = 0;
    }
    currentDisplayedCounter = 0;
  }
}






