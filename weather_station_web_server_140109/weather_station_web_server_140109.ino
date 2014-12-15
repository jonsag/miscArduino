
#include <SPI.h> // ethernet libraries    
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetServer.h>
//#include <EthernetUdp.h>
#include <util.h>
#include <Average.h> // add library for calculating averages

///////////////////// ethernet settings
byte mac[] = {  
  0x90, 0xA2, 0xDA, 0x0F, 0x1E, 0xD0 }; // MAC address, printed on a sticker on the shield
IPAddress ip(192,168,10,29); // ethernet shields wanted IP
//IPAddress gateway(192,168,10,1); // internet access via router
//IPAddress subnet(255,255,255,0); //subnet mask
EthernetServer server(80); // this arduinos web server port
String readString; // string read from servers interface

// declare in/outputs
int setRainZero = 6;
int anemometer = 5;
int rainGauge = 4;
int rainLed = 2;
int vane = A2;

// #define ONE_WIRE_BUS 3 // oneWire bus bin

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

boolean blinkRainLed = false; 
boolean rainLedOn = false;

int length1 = 0;
int length2 = 0;

////////////////////////////// setup //////////////////////////////
void setup() {
  Serial.begin(9600);
  Serial.println("_140108_weather_station_web_server");

  Serial.println("Started serial communication");
   Serial.println("Waiting 5 seconds...");
  delay(5000);  //important on linux a serial port can lock up otherwise

  // declare digital inputs
  Serial.println("Initiating digital inputs...");
  pinMode(setRainZero, INPUT);
  pinMode(anemometer, INPUT);
  pinMode(rainGauge, INPUT);
  pinMode(rainLed, OUTPUT);

  Serial.println("Starting ethernet...");
  Ethernet.begin(mac, ip);
  Serial.println("Starting web server...");
  server.begin(); //start arduinos web server

  Serial.println("Start!");
  delay(500);
} // end of start routine

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
      rainIntensity = 3600 * 0.279 / ((currentMillis - lastRainMillis) / 1000);
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

  ///////////////////// webserver
  presentWebPage();

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
} // end of direction sub

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
} // end of blinking sub

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
    Serial.print(" mm/h    ");
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
} // end of serial printing

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
} // end of length sub

/////////////////////////////// present webpage ///////////////////////////////
void presentWebPage(void) {
  EthernetClient client = server.available(); // listen for incoming clients
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (readString.length() < 100) { //read char by char HTTP request 
          readString += c; //store characters to string
        }
        if (c == '\n') { //if HTTP request has ended
          Serial.println(readString); //print to serial monitor for debuging

            //now output HTML data header
          if(readString.indexOf('?') >=0) { //don't send new page
            client.println("HTTP/1.1 204 JS Arduino");
            client.println();
            client.println();  
          }
          else {

            // headers
            client.println("HTTP/1.1 200 OK"); //send new page
            client.println("Content-Type: text/html");
            client.println();
            client.println("<HTML>");
            client.println("<HEAD>");
            client.println("<TITLE>Weather station</TITLE>");
            client.println("</HEAD>");
            client.println("<BODY>");

            client.println("<H1>JS Weather Station</H1>");
            client.println("Part of jsPowerTempLog");
            client.println("<br><br>");
/*
            client.println("Line01");
            client.println("<br>");
            
            client.println("Line02");
            client.println("<br>");
            
            client.println("Line03");
            client.println("<br>");
            client.println("Line04");
            client.println("<br>");
            client.println("Line05");
            client.println("<br>");
            client.println("Line06");
            client.println("<br>");
            client.println("Line07");
            client.println("<br>");
            client.println("Line08");
            client.println("<br>");
            client.println("Line09");
            client.println("<br>");
            client.println("Line10");
            client.println("<br>");
*/
            client.println(vaneDirection[directionValue]);
            //client.println(vaneDegrees[directionValue]);
            //client.println(displayedVaneAverage);
            //client.println(windSpeed);
            //client.println(rainIntensity);
            //client.println((currentMillis - lastRainMillis) / 1000);
            //client.println(totalRain);

            /*
            client.print("Wind direction: ");
             client.print(vaneDirection[directionValue]);
             client.println("<br>");
             client.print("Wind direction degrees: ");
             client.print(vaneDegrees[directionValue]);
             client.print("Average wind degrees: ");
             client.print(displayedVaneAverage);
             client.println("<br><br>");
             
             client.print("Wind speed: ");
             //client.print(windSpeed);
             client.print(" m/s    ");
             client.println("<br><br>");
             
             client.print("Rain intensity: ");
             client.print(rainIntensity);
             client.print(" mm/h");
             client.println("<br>");
             client.print("Last tip was ");
             client.print((currentMillis - lastRainMillis) / 1000);
             client.print(" s ago");
             client.println("<br>");
             client.print("Rain total: ");
             client.print(totalRain);
             client.print(" mm");
             client.println("<br><br>");
             
             client.println("<a href=\"/?rainReset\" target=\"inlineframe\">Reset rain meter</a><br>");
             client.println("<IFRAME name=inlineframe style=\"display:none\" >");          
             client.println("</IFRAME><br>");
             */
            client.println("</BODY>");
            client.println("</HTML>");
          }

          ///////////////////// check if we should reset rain meter
          if(readString.indexOf("rainReset") > 0) { //checks for click resetting rain meter
            totalRain = 0;
            Serial.print("Resetting rain from webpage"); 
          }

          ///////////////////// check if this is a poll
          if(readString.indexOf("pollReset") > 0) { //checks for poll reset
            Serial.println("---Recieved pollReset");
          }

          readString=""; //clearing string for next read
          delay(1);
          client.stop(); //stopping client
        }
      }
    }
  }
} // end of web page












