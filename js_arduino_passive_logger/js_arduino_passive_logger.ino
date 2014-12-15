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
#include <EthernetUdp.h>
#include <util.h>
#include <Time.h>

// settings for timesync
#define TIME_MSG_LEN  11   // time sync to PC is HEADER followed by Unix time_t as ten ASCII digits
#define TIME_HEADER  'T'   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 


// set pin numbers:
const int buttonPinPulse = 6;    // number of the button pin to simulate pulses
const int ledPinYellow =  7;      // number of the yellow LED pin, status
const int ledPinGreen =  8;      // number of the green LED pin, polled
const int ledPinRed =  9;      // number of the red LED pin, pulse recieved

const int analogPin0 = A0; // declare the analog inputs
const int analogPin1 = A1;
const int analogPin2 = A2;

float sensorValue[] = { // sensorValue holds the analog inputs
  0, 1, 2, 3};

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

unsigned long syncMillis = 000000; // will store last time clock was synced
unsigned long elapsedSyncMillis = 000000; // how long since the clock was synced

unsigned long currentMillis = 000000; // will store millis()

unsigned long loopStart = 000000; // will store when program loop started
unsigned long loopTime = 000000; // will store how long a program loop takes

unsigned long syncStart = 000000; // will store when clock sync started
unsigned long syncElapsed = 000000; // will store how long a clock sync takes
unsigned long timeSkew = 0; // how much time differs since last clock sync in seconds
unsigned long timeNow = 0; // will store now()

unsigned long pulsesSinceStart = 000000; // various pulse counters, 1 pulse = 1/1000 kwh
unsigned int pulsesLastInterval = 0;
unsigned int pulsesThisInterval = 0;
unsigned int pulses = 0;

byte mac[] = {  
  0x90, 0xA2, 0xDA, 0x0C, 0x00, 0x76 }; // MAC address, printed on a sticker on the shield
//IPAddress ip(192,168,1,155); // ethernet shields wanted IP
//IPAddress gateway(192,168,1,1); // internet access via router
//IPAddress subnet(255,255,255,0); //subnet mask
EthernetServer server(80); // this arduinos web server port

String readString; // string read from servers interface

// settings for timesync
unsigned int localPort = 8888; // local port to listen for UDP packets
//IPAddress timeServer(192, 43, 244, 18); // time.nist.gov NTP server
IPAddress timeServer(130, 235, 20, 67); // ntp.lth.se
const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
EthernetUDP Udp; // A UDP instance to let us send and receive packets over UDP

void setup() {
  pinMode(buttonPinPulse, INPUT); // initialize the pushbuttons pins as inputs
  pinMode(ledPinYellow, OUTPUT); // initialize the LED pins as outputs
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinRed, OUTPUT);
  //Ethernet.begin(mac, ip, subnet, gateway); // start ethernet
  Ethernet.begin(mac); // start ethernet
  server.begin(); //start arduinos web server
  Serial.begin(9600); // start serial communication
  Udp.begin(localPort); // start UDP
}

//////////////////////////////////// main loop ////////////////////////////////////
void loop() {
  ///////////////////// checking some times
  currentMillis = millis(); // millis right now
  loopTime = (currentMillis - loopStart); // how long does a loop on this program take
  loopStart = millis(); // now we start timing the loop
  elapsedSyncMillis = (currentMillis - syncMillis); // how much time elapsed since last clock sync
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
          //Serial.println(readString); //print to serial monitor for debuging
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
            client.print("Current time is: "); // digital clock display of the current time
            client.print(year());
            client.print("-");
            if (month() < 10) {
              client.print("0");
            }
            client.print(month());
            client.print("-");
            if (day() < 10) {
              client.print("0");
            }
            client.print(day());
            client.print(" ");
            client.print(hour());
            client.print(":");
            if (minute() < 10) {
              client.print("0");
            }
            client.print(minute());
            client.print(":");
            if (second() < 10) {
              client.print("0");
            }
            client.print(second());
            client.println(" (EST)<br>");
            client.print("Unix time is: "); // print the function now()
            client.print(now());
            client.println("<br>"); 
            client.println("<br>");
            client.print("Sensor value 0 is: "); // sensor values
            client.print(sensorValue[0]);
            client.println("<br>");
            client.print("Sensor value 1 is: ");
            client.print(sensorValue[1]);
            client.println("<br>");
            client.print("Sensor value 2 is: ");
            client.print(sensorValue[2]);
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
          readString=""; //clearing string for next read
        }
      }
    }
  }
  ///////////////////// read buttons and analog inputs
  pulseButtonState = digitalRead(buttonPinPulse); // read the button for simulating a pulse
  sensorValue[0] = analogRead(analogPin0) * 100.0 / 1024.0; // convert the analog inputs to a sensible value
  sensorValue[1] = analogRead(analogPin1) * 100.0 / 1024.0;
  sensorValue[2] = analogRead(analogPin2) * 100.0 / 1024.0;
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
  if (timeStatus() == timeNotSet) { // sync if time is not set
    setTime();
  }
  else {
    if (elapsedSyncMillis > 86400000) { // sync if it is too long since last sync
      setTime();
    }
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

//////////////////////////////////// sync clock ////////////////////////////////////
void setTime() {
  syncMillis = currentMillis; // save the last time you synced time
  timeNow = now(); // time now
  syncStart = millis(); // start timing the sync    
  digitalWrite(ledPinYellow, HIGH); // turn LED on
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  delay(1000); // wait to see if a reply is available
  if ( Udp.parsePacket() ) { // We've received a packet, read the data from it
    Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer
    syncButtonState = 1;
  }
  Serial.print("---Synced time\t");
  //the timestamp starts at byte 40 of the received packet and is four bytes,
  // or two words, long. First, extract the two words:
  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;  
  //Serial.print("Seconds since Jan 1 1900 = " );
  //Serial.println(secsSince1900);
  Serial.print("Unix time: "); // now convert NTP time into everyday time
  const unsigned long seventyYears = 2208988800UL; // Unix time starts on Jan 1 1970. In seconds, that's 2208988800
  unsigned long epoch = secsSince1900 - seventyYears; // subtract seventy years
  Serial.print(epoch); // print Unix time
  Serial.print("\t");    
  Serial.print("UTC time: "); // print the hour, minute and second,UTC is the time at Greenwich Meridian (GMT)
  Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
  Serial.print(':');  
  if ( ((epoch % 3600) / 60) < 10 ) { // leading zero
    Serial.print('0');
  }
  Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
  Serial.print(':');
  if ( (epoch % 60) < 10 ) { // leading zero
    Serial.print('0');
  }
  Serial.println(epoch %60); // print the second
  //eventLog(); // write to eventLog
  syncElapsed = (millis() - syncStart); // stop timing of sync
  setTime(epoch); // set the time to the recieved answer
  adjustTime(3600); // adjust time to timezone
  timeSkew = (now() - timeNow - syncElapsed / 1000); // how much did time differ
  Serial.print("Time skew: ");
  Serial.println(timeSkew);  
  digitalWrite(ledPinYellow, LOW); // turn LED off
}

unsigned long sendNTPpacket(IPAddress& address) // send an NTP request to the time server at the given address
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE); // set all bytes in the buffer to 0
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  packetBuffer[12]  = 49; // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:         
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket();
}


