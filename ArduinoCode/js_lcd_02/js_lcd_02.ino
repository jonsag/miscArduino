#include <LiquidCrystal.h>

// my pinout from L (LCD pin) to A (Arduino pin):
// LCD pin 1: Vss --> to Arduino GND
// LCD pin 2: Vdd --> to Arduino +5V
// LCD pin 3: V0 (contrast) --> to GND (I chose the PWM pin 10, see below)
// LCD pin 4: RS (register select) --> to Arduino pin 11
// LCD pin 5: R/W- (read/write) --> to Arduino pin 2
// LCD pin 6: E (H/L enable) --> to Arduino pin 3
// LCD pin 7: DB0 (data bit 0) --> to Arduino PIN 4
// LCD pin 8: DB1 (data bit 1) --> to Arduino PIN 5
// LCD pin 9: DB2 --> to Arduino PIN 6
// LCD pin 10: DB3 --> to Arduino PIN 7
// LCD pin 11: DB4 --> to Arduino PIN 14
// LCD pin 12: DB5 --> to Arduino PIN 15
// LCD pin 13: DB6 --> to Arduino PIN 16
// LCD pin 14: DB7 --> to Arduino PIN 17
// LCD pin 15: A/Vee (backlight+) --> to a 4.2Vcc source (see documentation)
// LCD pin 16: K (backlight-) --> to Arduino GND


LiquidCrystal lcd(11,2,3, 4,5,6,7, 14,15,16,17);

int contrast = 10;                 // PWM pin (contrast level)
int led = 13;                      // LED pin (alive led)

char *boot = "(C) AlfonSoftWare   Winstar 2004A KS0066"
             "InterNational 2009  Arduino Diecimila   ";

void setup()                       // initialization
{
  Serial.begin(115200);

  pinMode(contrast, OUTPUT);
  pinMode(led, OUTPUT);

  analogWrite(contrast, 0);       // 0: maximum contrast  255: minimum
  digitalWrite(led, 1);

  lcd.clear();
  lcd.print(boot);
}


void loop()                        // main loop
{
  static int i=0, ctr=3;           // led blinking section
  i = (i+1)&31;
  digitalWrite(led, i>ctr);

  if(!Serial.available())          // no incoming text?
  {
    delay(15);
    return;
  }
  
  static char pos[80] =
  {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
  };
  
  static int con[9] = { 0, 30, 60, 90, 120, 150, 180, 210 };
 
  char buf[81], *ptr = buf, chr, flg = 0;
  int cur = 0;
  memset(buf, ' ', sizeof(buf));
  buf[80] = '\0';

  delay(15);                     // wait for all chars to be in (80 chars at 57600)
  
  while(Serial.available() > 0)  // for every character available:
  {
    chr = Serial.read();         // fetch next
    
    if(chr == '\n')              // simulate carriage returns
    {
      if(! flg)                  // ignore fake carriage returns
      {
        if(cur<20) cur=20;       // from 1st to 2nd line
        else if(cur<40) cur=40;  // from 2nd to 3rd line
        else if(cur<60) cur=60;  // from 3rd to 4th line
      }
    }
    flg = 0;
    
    if(chr >= 1 && chr <= 9)     // contrast setting characters
    {
      ctr = chr*3;
      analogWrite(contrast, con[chr-1]);
    }
    
    if(chr<' ' || cur>79)        // skip non-printable chars
      continue;                  // and out-of-area chars
    
    buf[pos[cur++]] = chr;       // store character
    if(!(cur % 20)) flg = 1;     // remember "end of line" condition
  }
  
  lcd.clear();
  lcd.print(buf);                // output the formatted screenshot

  Serial.print(buf);             // serial debug output
  Serial.print("\r\n\n");
}
