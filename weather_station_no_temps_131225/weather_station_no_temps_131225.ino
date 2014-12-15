#include <Average.h> // add library for calculating averages

// declare in/outputs
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

unsigned long currentMillis = 000000; // will store millis()

unsigned long anemometerMillis = 000000;
unsigned long anemometerAge = 000000;

unsigned long lastRainMillis = 000000;

unsigned long blinkMillis = 000000;
unsigned long blinkAge = 000000;

unsigned long serialPrintMillis = 000000;
unsigned long serialPrintAge = 000000;

boolean blinkRainLed = false; 
boolean rainLedOn = false;

////////////////////////////// setup //////////////////////////////
void setup() {
  delay(5000);

  Serial.begin(9600);
  Serial.println("_131225_weather_station_no_temps");
  Serial.println("Started serial communication after waiting 5 seconds");

  // declare digital inputs
  Serial.println("Starting digital inputs...");
  pinMode(anemometer, INPUT);
  pinMode(rainGauge, INPUT);
  pinMode(rainLed, OUTPUT);

    Serial.println("Waiting 5 seconds...");
  delay(5000);  //important on linux a serial port can lock up otherwise

  Serial.println("Starting...");
}

////////////////////////////// main loop //////////////////////////////
void loop() {
  currentMillis = millis();

  ///////////////////////// read inputs
  anemometerState = digitalRead(anemometer);
  rainGaugeState = digitalRead(rainGauge);
  vaneValue = analogRead(vane);

  ///////////////////////// vane - wind direction
  directionValue = windDirection();

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

  ///////////////////// blink
  if (blinkRainLed || rainLedOn) { // if we should light led or if it's already lit
    rainBlinking();
  }

  ///////////////////// print to serial
  serialPrint(); // print output to serial

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
  if(serialPrintAge >= 10000) {
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


