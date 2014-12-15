#include <OneWire.h> // oneWire libraries
#include <DallasTemperature.h>

///////////////////// oneWire settings
#define ONE_WIRE_BUS 4 // oneWire bus bin
OneWire oneWire(ONE_WIRE_BUS); // setup oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire); // pass our oneWire reference to Dallas Temperature
byte numberOfSensors; // will store how many sensors we have
float tempValue[] = { // will store the temps
  0, 1, 2, 3};
boolean tempsRead = false; // will be true after first temp reading
byte present = 0;
byte data[12];
byte addr[8];
byte count = 0;
int i = 0; // counter

// declare inputs
int anemometer = 2;
int rainGauge = 3;
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

unsigned long tempsMillis = 000000; // will store last time temps was read
unsigned long tempAge = 000000; // how long since the temps was read

unsigned long anemometerMillis = 000000;
unsigned long anemometerAge = 000000;

unsigned long lastRainMillis = 000000;

////////////////////////////// setup //////////////////////////////
void setup() {

  Serial.begin(9600);
  Serial.println("_131219_weather_station");
  Serial.println("Started serial communication");

  // declare digital inputs
  Serial.println("Starting digital inputs...");
  pinMode(anemometer, INPUT);
  pinMode(rainGauge, INPUT);

  Serial.println("Starting 1-wire sensors...");
  sensors.begin(); // start up the oneWire library
  Serial.println("Discovering and counting sensors...");
  numberOfSensors = discoverOneWireDevices(); // count sensors

    Serial.println("Waiting 5 seconds...");
  delay(5000);  //important on linux a serial port can lock up otherwise

  Serial.println("Starting...");
}

////////////////////////////// main loop //////////////////////////////
void loop() {
  currentMillis = millis();
  //Serial.print(currentMillis);
  //Serial.print("    ");

  anemometerState = digitalRead(anemometer);
  rainGaugeState = digitalRead(rainGauge);
  vaneValue = analogRead(vane);

  ///////////////////////// vane - wind direction
  //Serial.print(vaneValue);
  //Serial.print("    ");

  directionValue = windDirection();
  Serial.print("Wind direction: ");
  Serial.print(vaneDirection[directionValue]);
  Serial.print("    ");
  Serial.print(vaneDegrees[directionValue]);
  Serial.print("    ");
  //Serial.print("°    ");

  ///////////////////////// anemometer - wind speed
  //Serial.print(anemometerState);
  //Serial.print("    ");

  anemometerAge = (currentMillis - anemometerMillis);

  if(anemometerState != lastAnemometerState) {
    anemometerPulses++;
  }
  lastAnemometerState = anemometerState;

  if(anemometerAge >= 1000) {
    //anemometerPulsesPerSecond = anemometerPulses;
    windSpeed = anemometerPulses * 0.667;
    anemometerPulses = 0;
    anemometerMillis = currentMillis;
  }

  //Serial.print(anemometerPulsesPerSecond);
  //Serial.print("    ");

  Serial.print("Wind speed: ");
  Serial.print(windSpeed);
  Serial.print(" m/s    ");


  ///////////////////////// rain gauge - tip bucket
  //Serial.print(rainGaugeState);
  //Serial.print("    ");

  if(rainGaugeState != lastRainGaugeState) {
    if(rainGaugeState == HIGH) {
      rainBucketTips++;
      totalRain = totalRain + 0.279;
      rainIntensity = 60 * 0.279 / ((currentMillis - lastRainMillis) / 1000);
      lastRainMillis = currentMillis;        
    }
  }  
  lastRainGaugeState = rainGaugeState;

  //Serial.print(rainBucketTips);
  //Serial.print("    ");
  Serial.print("Rain total: ");
  Serial.print(totalRain);
  Serial.print(" mm    ");

  //Serial.print(currentMillis - lastRainMillis);
  //Serial.print("    ");

  if((currentMillis - lastRainMillis) > 60000) {
    rainIntensity = 0;
  }
  Serial.print("Rain intensity: ");
  Serial.print(rainIntensity);
  Serial.print(" mm/min    ");

  ///////////////////// check if we should read temps
  tempAge = (currentMillis - tempsMillis); // how much time elapsed since last temp reading
  if (!tempsRead || tempAge > 10000) { // read temperatures if we haven't before, or if it's time to do so
    getTemperatures();
    tempsMillis = currentMillis; // save the last time you synced time
  }
  for(i=0; i < numberOfSensors; i++) {
    Serial.print("Temperature ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(tempValue[i]);
    Serial.print(" C");
  }
  Serial.println("    ");
}

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

/////////////////////////////// discover sensors ////////////////////////////////
int discoverOneWireDevices(void) {
  while(oneWire.search(addr)) {
    Serial.print("Found \'1-Wire\' device with address: ");
    for( i = 0; i < 8; i++) {
      Serial.print("0x");
      if (addr[i] < 16) {
        Serial.print('0');
      }
      Serial.print(addr[i], HEX);
      if (i < 7) {
        Serial.print(", ");
      }
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return 0;
    }
    Serial.println();
    count++;
  }
  oneWire.reset_search();
  return count;
}

/////////////////////////////// request temperatures ///////////////////////////////
void getTemperatures(void) {
  if(numberOfSensors != 0) {
    sensors.requestTemperatures(); // Send the command to get temperatures
    for(i=0; i < numberOfSensors; i++) { // read each of our sensors and print the value
      tempValue[i] = sensors.getTempCByIndex(i);
      tempsRead = true;
    }
  }
  else {
    Serial.println();
    Serial.println("No temperature sensors present");
  }
}
















