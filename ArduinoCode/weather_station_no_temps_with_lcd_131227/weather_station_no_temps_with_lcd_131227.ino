/*
 The circuit:
 *                             GND            display pin 1
 *  supply voltage for logic   +5V            display pin 2
 
 * 10K resistor:
 * ends to +5V and ground
 * LCD VO - contrast           wiper          display pin 3
 
 * LCD RS pin                  digital pin 12 display pin 4
 * LCD R/W pin                 GND            display pin 5
 * LCD Enable pin              digital pin 11 display pin 6
 * LCD D4                      digital pin 10 display pin 11
 * LCD D5                      digital pin 9  display pin 12
 * LCD D6                      digital pin 8  display pin 13
 * LCD D7                      digital pin 7  display pin 14
 
 * 10K resistor:
 * ends to +5V and ground
 * back light anode:+4.2V      wiper          display pin 15
 * back light cathode          wiper          display pin 16
 */

#include <Average.h> // add library for calculating averages
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

// declare in/outputs
int setRainZero = 1;
int anemometer = 2;
int rainGauge = 3;
int rainLed = 13;
int vane = A2;

int anemometerState = 0;
int lastAnemometerState = 0;
int anemometerPulses = 0;
//int anemometerPulsesPerSecond = 0;
float windSpeed = 0; // the speed in m/s
int beaufort = 0;
char* windLabels[] = {
  "Calm", "Light air", "Light breeze", "Gentle breeze", "Moderate breeze", "Fresh breeze", "Strong breeze", "High wind, moderate gale, near gale", "Gale, fresh gale", "Strong gale", "Storm, whole gale", "Violent storm", "Hurricane force"};
char* svWindLabels[] = {
  "Lugnt", "Svag vind", "Svag vind", "Måttlig vind", "Måttlig vind", "Frisk vind", "Frisk vind", "Hård vind", "Hård vind", "Mycket hård vind", "Storm", "Svår storm", "Orkan"};
char* svWindLabelsAtSea[] = {
  "Stiltje, bleke", "Nästan stiltje", "Lätt (laber) bris", "God bris", "Frisk bris", "Styv bris", "Hård bris, frisk kuling/kultje", "Styv kuling/kultje", "Hård kuling/kultje", "Halv storm", "Storm", "Svår storm", "Orkan"};

int setRainZeroState = 0;
int rainGaugeState = 0;
int lastRainGaugeState = 0;
int rainBucketTips = 0;
float totalRain = 0;
int rainIntensity = 0;

int vaneValue = 0;
int directionValue = 0;
char* vaneDirection[]={
  "NA ", "E  ", "N  ", "W  ", "S  ", "NE ", "NW ", "SW ", "SE ", "ENE", "NNE", "NNW", "WNW", "WSW", "SSW", "SSE", "ESE"};
char* vaneDegrees[]={
  "NA", "90", "0", "270", "180", "45", "315", "225", "135", "67.5", "22.5", "337.5", "292.5", "247.5", "202.5", "157.5", "112.5"};
int vaneAverageCounter = 0;
#define vaneSamples 100
int vaneAverage[vaneSamples];
int displayedVaneAverage = 0;

unsigned long currentMillis = 000000; // will store millis()

unsigned long anemometerMillis = 000000;
unsigned long anemometerAge = 000000;

unsigned long lastRainMillis = 000000;

unsigned long blinkMillis = 000000;
unsigned long blinkAge = 000000;

unsigned long serialPrintMillis = 000000;
unsigned long serialPrintAge = 000000;

unsigned long lcdPrintMillis = 000000;
unsigned long lcdPrintAge = 000000;

boolean blinkRainLed = false; 
boolean rainLedOn = false;

int length1 = 0;
int length2 = 0;

////////////////////////////// setup //////////////////////////////
void setup() {
  // set up the LCD's number of columns and rows:
  Serial.println("Initializing LCD...");
  lcd.begin(20, 4);
  // Print a message to the LCD.
  lcd.print("Booting...");

  delay(5000);
  Serial.begin(9600);
  Serial.println("_131227_weather_station_no_temps_with_lcd");
  lcd.setCursor(0, 1);
  lcd.print("Started serial");
  Serial.println("Started serial communication after waiting 5 seconds");
  lcd.setCursor(0, 2);
  lcd.print("Waiting...");
  Serial.println("Waiting another 5 seconds...");
  delay(5000);  //important on linux a serial port can lock up otherwise

  // declare digital inputs
  lcd.setCursor(0, 2);
  lcd.print("Starting in/outputs...");
  Serial.println("Starting digital inputs...");
  pinMode(setRainZero, INPUT);
  pinMode(anemometer, INPUT);
  pinMode(rainGauge, INPUT);
  pinMode(rainLed, OUTPUT);

  lcd.setCursor(0, 3);
  lcd.print("Starting now...");
  Serial.println("Start!");
  delay(500);
  lcd.clear();
}

////////////////////////////// main loop //////////////////////////////
void loop() {
  currentMillis = millis();

  ///////////////////////// read inputs
  setRainZeroState = digitalRead(setRainZero);
  anemometerState = digitalRead(anemometer);
  rainGaugeState = digitalRead(rainGauge);
  vaneValue = analogRead(vane);

  ///////////////////////// vane - wind direction
  directionValue = windDirection();
  if (vaneAverageCounter >= vaneSamples) {
    displayedVaneAverage = mean(vaneAverage,vaneAverageCounter),DEC;
    vaneAverageCounter = 0;
  }
  else {
    vaneAverage[vaneAverageCounter] = atoi(vaneDegrees[directionValue]);
    vaneAverageCounter++;
  }

  ///////////////////////// anemometer - wind speed
  anemometerAge = (currentMillis - anemometerMillis);

  if(anemometerState != lastAnemometerState) { // if we have recieved a pulse
    if(anemometerState == HIGH) { // if it went high
      anemometerPulses++;
      blinkRainLed = true;
    }
  }
  lastAnemometerState = anemometerState; // save state to next run

  if(anemometerAge >= 10000) {
    //anemometerPulsesPerSecond = anemometerPulses;
    windSpeed = anemometerPulses * 0.667 / 10; // one pulse is 0.667 m/s
    anemometerPulses = 0;
    anemometerMillis = currentMillis;
  }

  ///////////////////////// rain gauge - tip bucket
  if(rainGaugeState != lastRainGaugeState) { // if we have recieved a pulse
    if(rainGaugeState == HIGH) { // if it went high
      rainBucketTips++;
      totalRain = totalRain + 0.279; // one pulse is 0.279 mm of rain
      rainIntensity = 60 * 0.279 / ((currentMillis - lastRainMillis) / 1000);
      lastRainMillis = currentMillis;
      blinkRainLed = true;     
    }
  }  
  lastRainGaugeState = rainGaugeState; // save state for next run

  if((currentMillis - lastRainMillis) > 60000) { // if we haven't recieved a puse for 60 seconds, set intensity to 0
    rainIntensity = 0;
  }

  //if (setRainZeroState) {
  //Serial.println("Resetting rain meter");
  // totalRain = 0;
  //blinkRainLed = true;
  //}

  ///////////////////// blink
  if (blinkRainLed || rainLedOn) { // if we should light led or if it's already lit
    rainBlinking();
  }

  ///////////////////// print to serial
  serialPrint(); // print output to serial
  lcdPrint();

} // end of main loop

////////////////////////////// evaluate wind direction //////////////////////////////
int windDirection(void) {
  if(vaneValue >= 0 && vaneValue <= 50) { // ESE
    directionValue = 16;
  }
  else if(vaneValue >= 51 && vaneValue <= 59) { // ENE
    directionValue = 9;
  }
  else if(vaneValue >= 60 && vaneValue <= 75) { // E
    directionValue = 1;
  }
  else if(vaneValue >= 76 && vaneValue <= 115) { // SSE
    directionValue = 15;
  }
  else if(vaneValue >= 116 && vaneValue <= 155) { // SE
    directionValue = 8;
  }
  else if(vaneValue >= 156 && vaneValue <= 190) { // SSW
    directionValue = 14;
  }
  else if(vaneValue >= 191 && vaneValue <= 270) { // S
    directionValue = 4;
  }
  else if(vaneValue >= 271 && vaneValue <= 335) { // NNE
    directionValue = 10;
  }
  else if(vaneValue >= 336 && vaneValue <= 450) { // NE
    directionValue = 5;
  }
  else if(vaneValue >= 451 && vaneValue <= 510) { // WSW
    directionValue = 13;
  }
  else if(vaneValue >= 511 && vaneValue <= 570) { // SW
    directionValue = 7;
  }
  else if(vaneValue >= 571 && vaneValue <= 660) { // NNW
    directionValue = 11;
  }
  else if(vaneValue >= 661 && vaneValue <= 730) { // N
    directionValue = 2;
  }
  else if(vaneValue >= 731 && vaneValue <= 805) { // WNW
    directionValue = 12;
  }
  else if(vaneValue >= 806 && vaneValue <= 870) { // NW
    directionValue = 6;
  }
  else if(vaneValue >= 871 && vaneValue <= 1023) { // W
    directionValue = 3;
  }
  else {
    directionValue = 0;
  }
  return directionValue;
}

/////////////////////////////// blinking rain LED ///////////////////////////////
void rainBlinking(void) {
  blinkAge = blinkMillis - currentMillis;
  if (rainLedOn && blinkAge >= 200) {
    digitalWrite(rainLed, LOW);
    rainLedOn = false;
  }
  else if(blinkRainLed) {
    digitalWrite(rainLed, HIGH);
    blinkMillis = currentMillis;
    blinkRainLed = false;
    rainLedOn = true;
  }
}

/////////////////////////////// print to serial ///////////////////////////////
void serialPrint(void) {
  serialPrintAge = currentMillis - serialPrintMillis;
  if(serialPrintAge >= 1000) {
    //Serial.print(currentMillis);
    //Serial.print("    ");

    //Serial.print(vaneValue);
    //Serial.print("    ");
    Serial.print("Wind direction: ");
    Serial.print(vaneDirection[directionValue]);
    Serial.print("    ");
    Serial.print(vaneDegrees[directionValue]);
    Serial.print("    ");

    //Serial.print(anemometerState);
    //Serial.print("    ");
    //Serial.print(anemometerPulsesPerSecond);
    //Serial.print("    ");
    Serial.print("Wind speed: ");
    Serial.print(windSpeed);
    Serial.print(" m/s    ");

    Serial.print("Rain intensity: ");
    Serial.print(rainIntensity);
    Serial.print(" mm/min    ");
    //Serial.print(rainGaugeState);
    //Serial.print("    ");
    //Serial.print(rainBucketTips);
    //Serial.print("    ");
    Serial.print("Rain total: ");
    Serial.print(totalRain);
    Serial.print(" mm    ");
    //Serial.print(currentMillis - lastRainMillis);
    //Serial.print("    ");

    Serial.println();

    serialPrintMillis = currentMillis;
  }
}

/////////////////////////////// print to LCD ///////////////////////////////
void lcdPrint(void) {
  lcd.setCursor(0, 0);
  lcd.print(currentMillis);
  lcdPrintAge = currentMillis - lcdPrintMillis;
  if(lcdPrintAge >= 1000) {
    ///// print wind
    lcd.setCursor(0, 1);
    lcd.print(vaneDirection[directionValue]);

    //length = getLength(vaneDirection[directionValue]);
    lcd.setCursor(4, 1);
    lcd.print("   ");
    length1 = getLength(displayedVaneAverage);
    lcd.setCursor(7 - length1, 1);
    lcd.print(displayedVaneAverage);

    lcd.setCursor(7, 1);
    lcd.print((char)223);

    lcd.setCursor(12, 1);
    //windSpeed = 8.43;
    lcd.print(windSpeed);
    //length = 3 + getLength(windSpeed);
    length2 = 17;
    lcd.setCursor(length2, 1);
    lcd.print("m/s");

    ///// print rain
    lcd.setCursor(0, 2);
    lcd.print("Rain: ");
    lcd.setCursor(6, 2);
    lcd.print(totalRain);

    lcdPrintMillis = currentMillis;
  }
}

/////////////////////////////// get length ///////////////////////////////
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








