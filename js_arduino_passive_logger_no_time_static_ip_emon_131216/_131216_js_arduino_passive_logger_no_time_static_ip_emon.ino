/*
Define these variables down in the code:
 ------------------------------------
 byte mac[] = -----the hex numbers on the sticker on your ethernet shield
 IPAddress ip( ----- the IP number you want yor arduino board to use
 if ( pollAge > 600000 ----- how long is the intervals between automatic reset of puls counting, milliseconds
 if (!tempsRead || tempAge > 60000 -----how often should we reset temps, milliseconds
 
 intervals - millis() - milliseconds:
 ------------------------------------
 10 seconds = 10000
 1 min = 60000
 5 min = 300000
 10 min = 600000
 1 hour = 3600000
 1 day = 86400000
 1 week = 604800000
 
 LEDs
 ------------------------------------
 YELLOW - discovering or reading temp sensors
 GREEN - activity on internet interface
 RED - pulse recieved
 */

#include <SPI.h> // ethernet libraries    
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetServer.h>
//#include <EthernetUdp.h>
#include <OneWire.h> // oneWire libraries
#include <DallasTemperature.h>
#include <util.h>
#include "EmonLib.h" // Include Emon Library
//#include <Time.h> // time library

///////////////////// ethernet settings
byte mac[] = {  
  0x90, 0xA2, 0xDA, 0x0C, 0x00, 0x76 }; // MAC address, printed on a sticker on the shield
IPAddress ip(192,168,10,10); // ethernet shields wanted IP
//IPAddress gateway(192,168,10,1); // internet access via router
//IPAddress subnet(255,255,255,0); //subnet mask

EthernetServer server(80); // this arduinos web server port
String readString; // string read from servers interface

///////////////////// oneWire settings
#define ONE_WIRE_BUS 2 // oneWire bus bin
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

///////////////////// set pin numbers:
const int buttonPinPulse = 3;    // number of the button pin to simulate pulses
const int pinPulse = 4; // number of the pin connected to the photo transistor
const int yellowPinTemp =  7;      // number of the yellow LED pin, status
const int greenPinNetwork =  8;      // number of the green LED pin, polled
const int redPinPulse =  9;      // number of the red LED pin, pulse recieved

const int current1 = A2; // declare the analog inputs
const int current2 = A3;
const int current3 = A4;

EnergyMonitor emon1;  // Create current instances
EnergyMonitor emon2;
EnergyMonitor emon3;

//float current[] = { // holds the analog inputs
//  1, 2, 3};
//float lastCurrent[] = { // the analog input from last run
//  1, 2, 3};
float displayedCurrent[] = { // the displayed value on the web page
  1, 2, 3};
float ackDisplayedCurrent[] = { // all reads ackumulated
  1, 2, 3};
float polledCurrent[] = { // the average current between each poll
  1, 2, 3};
float ackPolledCurrent[] = { // all averages ackumulated
  1, 2, 3};

///////////////////// variables
int pulseButtonState = 0; // variable for reading pulse button status
int lastPulseButtonState = 0; // saves pulse buttons previous state
int pulseState = 0;
int lastPulseState = 0;
int syncButtonState = 0; // variable for reading sync button status

int ledStateYellow = LOW; // ledState holds the LED outputs
int ledStateGreen = LOW;
int ledStateRed = LOW;

boolean blinkRed = false; // blink the LEDs
boolean blinkGreen = false;
boolean blinkYellow = false;

int blinkRedCounter = 0; // counters for blinks of the LEDs
int blinkGreenCounter = 0;
int blinkYellowCounter = 0;
int blinkYellowOffCounter = 0;

unsigned long currentMillis = 000000; // will store millis()

unsigned long pollMillis = 000000; // will store last time resetInterval was reset
unsigned long pollAge = 000000; // how long since last poll

unsigned long tempsMillis = 000000; // will store last time temps was read
unsigned long tempAge = 000000; // how long since the temps was read

unsigned int pulsesLastPoll = 0; // various pulse counters, 1 pulse = 1/1000 kwh
unsigned int pulses = 0;

///////////////////// counter for how many readings should be averaged
//int readAverageCounter = 200;

///////////////////// counters
byte i = 0; // counter for different purposes
byte poll = 0; // counter in the web poll
byte webDisplay = 0; // counter for displaying values on web page
byte addPolledCurrents = 0; // counter for the adding of polled currents
byte addCurrents = 0; // counter for the adding of displayed currents
int currentDisplayedCounter = 0; // counter for displayed current average
unsigned long currentPollCounter = 0; // counter för current interval average
int readCounter = 0; // counter for how often to read currents
boolean readTemps = false;

//////////////////////////////////// setup ////////////////////////////////////
void setup() {
  Serial.begin(9600); // start serial communication
  Serial.println("js_arduino_passive_logger_no_time_static_ip");
  Serial.println("Started serial communication");
  
  Serial.println("Initiating in/outputs...");
  pinMode(buttonPinPulse, INPUT); // initialize the pushbuttons pins as inputs
  pinMode(pinPulse, INPUT);
  pinMode(yellowPinTemp, OUTPUT); // initialize the LED pins as outputs
  pinMode(greenPinNetwork, OUTPUT);
  pinMode(redPinPulse, OUTPUT);
  
  Serial.println("Starting 1-wire sensors...");
  sensors.begin(); // start up the oneWire library
  Serial.println("Discovering and counting sensors...");
  numberOfSensors = discoverOneWireDevices(); // count sensors
  delay(5000);  //important on linux a serial port can lock up otherwise
  
  Serial.println("Starting ethernet...");
  Ethernet.begin(mac, ip);
  Serial.println("Starting web server...");
  server.begin(); //start arduinos web server
  
  Serial.println("Declaring and calibrating input pins for current measuring...");
  emon1.current(2, 49.4); // Current: input pin, calibration.
  emon2.current(3, 49.4);
  emon3.current(4, 49.4);

  //for(i=1; i <= 3; i++) { // setting some variables to 0
  // lastCurrent[i] = 0;
  // polledCurrent[i] = 0;
  //}
}

//////////////////////////////////// main loop ////////////////////////////////////
void loop() {

  ///////////////////// checking some times
  currentMillis = millis(); // millis right now
  pollAge = (currentMillis - pollMillis); // how much time elapsed since last log write
  tempAge = (currentMillis - tempsMillis); // how much time elapsed since last temp reading

  ///////////////////// read digital inputs
  pulseButtonState = digitalRead(buttonPinPulse); // read the button for simulating a pulse
  pulseState = digitalRead(pinPulse); // read pulse input

  ///////////////////// read analog inputs
  //if (!readTemps && readCounter >= 200) { // if we are not reading temps and just this often
  if (!readTemps) {
    readCurrents();
    // readCounter = 0; // reset counter
  }
  // else {
  // readCounter++;
  // }

  ///////////////////// check for button simulating a pulse
  if (pulseButtonState != lastPulseButtonState) { // light red LED if button is pressed
    if (pulseButtonState == HIGH) { // if the current state is HIGH then the button went from off to on
      pulses++; // +1 on pulse counter
      Serial.print("Pulse from button, pulses this interval is now "); 
      Serial.println(pulses);
      blinkRed = true;
    }
  }
  lastPulseButtonState = pulseButtonState; // save pulse button state till next loop

  ///////////////////// check for pulse
  if (pulseState != lastPulseState) { // light red LED if button is pressed
    if (pulseState == HIGH) { // if the current state is HIGH then the button went from off to on
      pulses++; // +1 on puls counter
      Serial.print("Pulse recieved, pulses this interval is now "); 
      Serial.println(pulses);
      blinkRed = true;
    }
  }
  lastPulseState = pulseState; // save pulse button state till next loop

  ///////////////////// check if it's time to reset pulses
  if ( pollAge > 600000) {
    pollMillis = currentMillis;
    Serial.println("---Resetting pulses");
    pulsesLastPoll = pulses;
    pulses = 0;
  }

  ///////////////////// check if we should read temps
  if (!tempsRead || tempAge > 60000) { // read temperatures if we haven't before, or if it's time to do so
    readTemps = true;
    getTemperatures();
  }

  ///////////////////// blink LEDs
  if (blinkYellow && blinkYellowCounter < 20) {
    blinkYellowCounter++;
    digitalWrite(yellowPinTemp, HIGH);
  }
  else {
    blinkYellow = false;
    blinkYellowCounter = 0;
    digitalWrite(yellowPinTemp, LOW);
  }
  if (!blinkYellow && blinkYellowOffCounter < 10000) {
    blinkYellowOffCounter++;
  }
  else {
    blinkYellow = true;
    blinkYellowOffCounter = 0;
  }
  if (blinkGreen && blinkGreenCounter < 20) {
    blinkGreenCounter++;
    digitalWrite(greenPinNetwork, HIGH);
  }
  else {
    blinkGreen = false;
    blinkGreenCounter = 0;
    digitalWrite(greenPinNetwork, LOW);
  }
  if (blinkRed  && blinkRedCounter < 20) {
    blinkRedCounter++;
    digitalWrite(redPinPulse, HIGH);
  }
  else {
    blinkRed = false;
    blinkRedCounter = 0;
    digitalWrite(redPinPulse, LOW);
  }

  ///////////////////// webserver
  presentWebPage();

}

/////////////////////////////// discover sensors ////////////////////////////////
int discoverOneWireDevices(void) {
  digitalWrite(yellowPinTemp, HIGH); // turn yellow LED on
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
  digitalWrite(yellowPinTemp, LOW); // turn yellow LED off
}

/////////////////////////////// request temperatures ///////////////////////////////
void getTemperatures(void) {
  digitalWrite(yellowPinTemp, HIGH); // turn yellow LED on
  tempsMillis = currentMillis; // save the last time you synced time
  Serial.println("---Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  for(i=0; i < numberOfSensors; i++) { // read each of our sensors and print the value
    Serial.print("Temperature for Device ");
    Serial.print( i );
    Serial.print(" is: ");
    tempValue[i] = sensors.getTempCByIndex(i);
    Serial.println(tempValue[i]);
  }
  tempsRead = true;
  digitalWrite(yellowPinTemp, LOW); // turn yellow LED off
  readTemps = false;
}

/////////////////////////////// present webpage ///////////////////////////////
void presentWebPage(void) {
  EthernetClient client = server.available(); // listen for incoming clients
  if (client) {
    digitalWrite(greenPinNetwork, HIGH); // turn green LED on
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
            client.println("<TITLE>Arduino logger</TITLE>");
            client.println("</HEAD>");
            client.println("<BODY>");
            client.println("<H1>JS temp and power consumption logger</H1>");

            // currents
            client.println("<br>");
            for(webDisplay=1; webDisplay <= 3; webDisplay++) {
              client.print("Current phase ");
              client.print(webDisplay);
              client.print(": ");
              client.print(displayedCurrent[webDisplay]);
              //client.print(current[i]);
              client.println(" A<br>");
            }
            client.println("<br>");
            for(webDisplay=1; webDisplay <= 3; webDisplay++) {
              client.print("Current average phase ");
              client.print(webDisplay);
              client.print(": ");
              client.print(polledCurrent[webDisplay]);
              //client.print((ackPolledCurrent[i] / currentPollCounter) / 100);
              client.println(" A<br>");
            }
            client.print("Based on ");
            client.print(currentPollCounter);
            client.println(" values<br>");
            client.print("Polled ");
            client.print(pollAge/1000);
            client.println(" seconds ago");
            client.println("<br><br>");

            // temps
            for(webDisplay=0; webDisplay < numberOfSensors; webDisplay++) {
              client.print("Temperature sensor ");
              client.print(webDisplay);
              client.print(": ");
              client.print(tempValue[webDisplay]);
              client.println(" C<br>");
            }
            client.print("Temps read ");
            client.print(tempAge/1000);
            client.println(" seconds ago<br>");
            client.println("<br>");

            // pulses
            client.print("Pulses last poll: ");
            client.print(pulsesLastPoll);
            client.println("<br>");
            client.print("Pulses since last poll: ");
            client.print(pulses);
            client.println("<br>");

            client.println("<br>");
            client.println("<a href=\"/?pulse\" target=\"inlineframe\">Send pulse</a><br>");
            client.println("<IFRAME name=inlineframe style=\"display:none\" >");          
            client.println("</IFRAME><br>");
            client.println("</BODY>");
            client.println("</HTML>");
          }

          ///////////////////// check if we should add a pulse
          if(readString.indexOf("pulse") > 0) { //checks for click simulating a pulse
            pulses++;
            Serial.print("Pulse from webpage, pulses this interval is now "); 
            Serial.println(pulses);
            blinkRed = true;
          }

          ///////////////////// check if this is a poll
          if(readString.indexOf("pollReset") > 0) { //checks for poll reset
            Serial.println("---Recieved pollReset, will reset pulses and average current values");
            pollMillis = currentMillis;
            pulsesLastPoll = pulses;
            pulses =0;
            currentPollCounter = 0;
            for(poll=1; poll <= 3; poll++) {
              ackPolledCurrent[poll] = 0;
            }
          }

          ///////////////////// check if we should read temps
          if(readString.indexOf("readTemps") > 0) { //will read 1-wire temps
            getTemperatures();
          }

          readString=""; //clearing string for next read
          delay(100);
          client.stop(); //stopping client
          digitalWrite(greenPinNetwork, HIGH); // turn green LED on
        }
      }
    }
  }
}

/////////////////////////////// read currents ///////////////////////////////
void readCurrents(void) {
  double Irms1 = emon1.calcIrms(200);  // Calculate Irms only
  double Irms2 = emon2.calcIrms(200);
  double Irms3 = emon3.calcIrms(200);
  
  displayedCurrent[1] = Irms1;
  displayedCurrent[2] = Irms2;
  displayedCurrent[3] = Irms3;
  
  Serial.print("1: ");
  Serial.print(Irms1*230.0);	       // Apparent power
  Serial.print(" ");
  Serial.print(Irms1);		       // Irms
  Serial.print("    ");

  Serial.print("2: ");
  Serial.print(Irms2*230.0);	       // Apparent power
  Serial.print(" ");
  Serial.print(Irms2);		       // Irms
  Serial.print("    ");

  Serial.print("1: ");
  Serial.print(Irms3*230.0);	       // Apparent power
  Serial.print(" ");
  Serial.print(Irms3);		       // Irms
  Serial.println("    ");
  /*
  if (currentDisplayedCounter < 200) {
   ackDisplayedCurrent[1] = ackDisplayedCurrent[1] + abs((analogRead(current1) - 511) * 0.33); // read currents
   ackDisplayedCurrent[2] = ackDisplayedCurrent[2] + abs((analogRead(current2) - 511) * 0.33);
   ackDisplayedCurrent[3] = ackDisplayedCurrent[3] + abs((analogRead(current3) - 511) * 0.33);
   currentDisplayedCounter++;
   }
   else {
   // Serial.print("displayedCurrent   ");
   for(addCurrents=1; addCurrents <= 3; addCurrents++) {
   displayedCurrent[addCurrents] = ackDisplayedCurrent[addCurrents] / 200; // set the displayed current value
   Serial.print(addCurrents);
   Serial.print(": ");
   Serial.print(displayedCurrent[addCurrents]);
   Serial.print(" A     ");
   ackDisplayedCurrent[addCurrents] = 0;
   }
   Serial.println();
   currentPollCounter++;
   currentDisplayedCounter = 0;   
   for(addPolledCurrents=1; addPolledCurrents <= 3; addPolledCurrents++) {
   ackPolledCurrent[addPolledCurrents] = ackPolledCurrent[addPolledCurrents] + displayedCurrent[addPolledCurrents]; // calculate an average for interval
   polledCurrent[addPolledCurrents] = ackPolledCurrent[addPolledCurrents] / currentPollCounter; // set the displayed current value
   }
   }
   */
}










