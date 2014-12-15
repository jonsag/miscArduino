/*
 intervals - millis() - milliseconds:
 ------------------------------------
 10 seconds = 10000
 1 min = 60000
 5 min = 300000
 10 min = 600000
 1 hour = 3600000
 1 day = 86400000
 1 week = 604800000
 */

#include <SPI.h>      
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetServer.h>
#include <util.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// set pin numbers:
const int buttonPinPulse = 6;    // number of the button pin to simulate pulses
const int ledPinYellow =  7;      // number of the yellow LED pin, status
const int ledPinGreen =  8;      // number of the green LED pin, polled
const int ledPinRed =  9;      // number of the red LED pin, pulse recieved

const int analogPin0 = A0; // declare the analog inputs
const int analogPin1 = A1;
const int analogPin2 = A2;

float sensorValue1; // sensorValue holds the analog inputs
float sensorValue2;
float sensorValue3;

// variables will change:
int pulseButtonState = 0; // variable for reading pulse button status
int lastPulseButtonState = 0; // saves pulse buttons previous state
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

unsigned long resetMillis = 000000; // will store last time resetInterval was reset
unsigned long elapsedResetMillis = 000000; // how long since resetMillis was reset

unsigned long currentMillis = 000000; // will store millis()

unsigned long pulsesSinceStart = 000000; // various pulse counters, 1 pulse = 1/1000 kwh
unsigned int pulsesLastInterval = 0;
unsigned int pulsesThisInterval = 0;
unsigned int pulses = 0;

// settings for 1-wire
#define ONE_WIRE_BUS 13 // where is data wire plugged into
#define TEMPERATURE_PRECISION 10 //resolution of the sensors is set to 10bit
OneWire oneWire(ONE_WIRE_BUS); // setup a 1-wire instance to communicate with any 1-wire devices
DallasTemperature sensors(&oneWire); // pass our 1-wire reference to dallas temperature
DeviceAddress temp1 = { 
  0x28, 0xAC, 0x59, 0xC3, 0x03, 0x00, 0x00, 0x0C };
DeviceAddress temp2 = { 
  0x28, 0x31, 0x77, 0xC3, 0x03, 0x00, 0x00, 0xAD };
float tempValue1; // tempValue holds the 1-wire temperatures
float tempValue2;

// IP settings
byte mac[] = {  
  0x90, 0xA2, 0xDA, 0x0C, 0x00, 0x76 }; // MAC address, printed on a sticker on the shield
//IPAddress ip(192,168,1,155); // ethernet shields wanted IP
//IPAddress gateway(192,168,1,1); // internet access via router
//IPAddress subnet(255,255,255,0); //subnet mask
EthernetServer server(80); // this arduinos web server port

String readString; // string read from servers interface

boolean voidLoop = false;

void setup() {
  Serial.begin(9600); // start serial communication
  Serial.println("js_arduino_passive_logger_with_1_wire_temps_without_clock");
  Serial.println("Started serial communication");
  Serial.println("Initializing inputs/outputs...");
  pinMode(buttonPinPulse, INPUT); // initialize the pushbuttons pins as inputs
  pinMode(ledPinYellow, OUTPUT); // initialize the LED pins as outputs
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinRed, OUTPUT);
  Serial.println("Starting ethernet...");
  //Ethernet.begin(mac, ip, subnet, gateway); // start ethernet
  Ethernet.begin(mac); // start ethernet
  Serial.println("Starting web server...");
  server.begin(); //start arduinos web server
  Serial.println("Starting 1-wire sensors...");
  //sensors.begin(); // start up 1-wire sensors
}

//////////////////////////////////// main loop ////////////////////////////////////
void loop() {
  if (!voidLoop) {
    Serial.println("Main loop started");
    voidLoop = true;
  }
  ///////////////////// checking some times
  elapsedResetMillis = (currentMillis - resetMillis); // how much time elapsed since last log write
  ///////////////////// webserver
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
            client.println("HTTP/1.1 200 OK"); //send new page
            client.println("Content-Type: text/html");
            client.println();
            client.println("<HTML>");
            client.println("<HEAD>");
            client.println("<TITLE>Arduino logger</TITLE>");
            client.println("</HEAD>");
            client.println("<BODY>");
            client.println("<H1>JS temp and power consumption logger</H1>");
            client.print("Sensor value 1 is: "); // sensor values
            client.print(sensorValue1);
            client.println("<br>");
            client.print("Sensor value 2 is: ");
            client.print(sensorValue2);
            client.println("<br>");
            client.print("Sensor value 3 is: ");
            client.print(sensorValue3);
            client.println("<br>");
            client.println("<br>");
            client.print("Pulses since start: "); // print pulses
            client.print(pulsesSinceStart);
            client.println("<br>");
            client.print("Pulses last interval: ");
            client.print(pulsesLastInterval);
            client.println("<br>");
            client.print("Pulses this interval: ");
            client.print(pulsesThisInterval);
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
          delay(1);
          client.stop(); //stopping client
          if(readString.indexOf("pulse") > 0) { //checks for click to simulating a pulse
            pulsesSinceStart++; // +1 on all pulse counters
            pulsesThisInterval++;
            pulses++;
            Serial.print("Pulse from webpage, pulses since start is now "); 
            Serial.println(pulsesSinceStart);
            blinkRed = true;
          }
          if(readString.indexOf("pollReset") > 0) { //checks for poll reset
            Serial.println("Recieved pollReset, will reset pulses");
            pulses =0;
            blinkGreen = true;
          }
          if(readString.indexOf("readTemps") > 0) { //will read 1-wire temps
            delay(1000);
            Serial.print("Getting temperatures...\n\r");
            sensors.requestTemperatures();
            float tempValue1 = sensors.getTempC(temp1);
            float tempValue2 = sensors.getTempC(temp2);
            Serial.print("Temp1: ");
            Serial.print(tempValue1);
            Serial.print("\t Temp2: ");
            Serial.println(tempValue2);
          }
          readString=""; //clearing string for next read
        }
      }
    }
  }
  ///////////////////// read buttons and analog inputs
  pulseButtonState = digitalRead(buttonPinPulse); // read the button for simulating a pulse
  sensorValue1 = analogRead(analogPin0) * 100.0 / 1024.0; // convert the analog inputs to a sensible value
  sensorValue2 = analogRead(analogPin1) * 100.0 / 1024.0;
  sensorValue3 = analogRead(analogPin2) * 100.0 / 1024.0;
  ///////////////////// check for button simulating a pulse
  if (pulseButtonState != lastPulseButtonState) { // light red LED if button is pressed
    if (pulseButtonState == HIGH) { // if the current state is HIGH then the button went from off to on
      pulsesSinceStart++; // +1 on all pulse counters
      pulsesThisInterval++;
      pulses++;
      Serial.print("Pulse from button, pulses since start is now "); 
      Serial.println(pulsesSinceStart);
      blinkRed = true;
    }
  }
  lastPulseButtonState = pulseButtonState; // save pulse button state till next loop
  ///////////////////// check if it's time to reset pulses
  if ( elapsedResetMillis > 60000) {
    resetMillis = currentMillis;
    Serial.println("---Resetting pulsesThisInterval");
    pulsesLastInterval = pulsesThisInterval;
    pulsesThisInterval = 0;
  }
  ///////////////////// blink LEDs
  if (blinkYellow && blinkYellowCounter < 20) {
    blinkYellowCounter++;
    digitalWrite(ledPinYellow, HIGH);
  }
  else {
    blinkYellow = false;
    blinkYellowCounter = 0;
    digitalWrite(ledPinYellow, LOW);
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
    digitalWrite(ledPinGreen, HIGH);
  }
  else {
    blinkGreen = false;
    blinkGreenCounter = 0;
    digitalWrite(ledPinGreen, LOW);
  }
  if (blinkRed  && blinkRedCounter < 20) {
    blinkRedCounter++;
    digitalWrite(ledPinRed, HIGH);
  }
  else {
    blinkRed = false;
    blinkRedCounter = 0;
    digitalWrite(ledPinRed, LOW);
  }
}






