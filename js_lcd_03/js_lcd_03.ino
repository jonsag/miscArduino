/*
  LiquidCrystal Library - Hello World
 
 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.
 
 This sketch prints "Hello World!" to the LCD
 and shows the time.
 
 The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin 11 - to digital pin 5
 * LCD D5 pin 12 - to digital pin 4
 * LCD D6 pin 13 - to digital pin 3
 * LCD D7 pin 14 - to digital pin 2
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 
 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe
 
 This example code is in the public domain.
 
 http://www.arduino.cc/en/Tutorial/LiquidCrystal
 */

// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

int length = 0;

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);
  // Print a message to the LCD.
  lcd.print("hello, world!");
}

void loop() {

  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):

  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  int time = millis() / 1000;
  lcd.print(time);
  length = getLength(time);
  lcd.setCursor(length, 1);
  lcd.print(" s since start    ");


}

int getLength(int value) { 
  int length;

  if(value > 9999)
    length = 5;
  else if(value > 999)
    length = 4;
  else if(value > 99)
    length = 3;
  else if(value > 9)
    length = 2;
  else
    length = 1;

  return length;
}




