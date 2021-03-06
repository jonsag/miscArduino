/*

 Comments
 
 */

#include <SPI.h>      
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <util.h>
#include <Time.h>
// #include <SD.h>

// settings for timesync
#define TIME_MSG_LEN  11   // time sync to PC is HEADER followed by Unix time_t as ten ASCII digits
#define TIME_HEADER  'T'   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 


// constants won't change. They're used here to set pin numbers:
const int buttonPin2 = 2;    // simulate pulses button
const int buttonPin3 = 3;    // the number of the pushbutton pin to sync clock
const int ledPin4 =  4;      // the number of the yellow LED pin
const int ledPin5 =  5;      // the number of the green LED pin
const int ledPin6 =  6;      // the number of the red LED pin

const int sensorPin0 = A0;
const int sensorPin1 = A1;
const int sensorPin2 = A2;
const int sensorPin3 = A3;
const int sensorPin4 = A4;
const int sensorPin5 = A5;

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status
int buttonState2 = 0;
int lastButtonState2 = 0;
int buttonState3 = 0;

int ledState4 = LOW;             // ledState4 used to set the LED
int ledState5 = LOW;
int ledState6 = LOW;

int sensorValue0 = 0;
int sensorValue1 = 0;
int sensorValue2 = 0;
int sensorValue3 = 0;
int sensorValue4 = 0;
int sensorValue5 = 0;

// reason for syncing clock
int syncReason = 0;
char* syncReasonText[] = {
  "initializing", "time not set", "sync time achieved", "button pressed"};

// reason for writing log
int logReason = 0;
char* logReasonText[] = {
  "initializing", "log_time_achieved"};

/* intervals
 1 min = 60000
 5 min = 300000
 10 min = 600000
 1 hour = 3600000
 1 day = 86400000
 1 week = 604800000 */

long blinkInterval = 10000; // interval at which to blink (milliseconds)

long logInterval = 60000; // how often to write log
long logMillis = 0; // will store last time log was written
long elapsedLogMillis = 0; // how long since log was written
long logLeft = 0; // variable for counting down to log write

long syncInterval = 3600000; // how often will we sync clock
long syncMillis = 0; // will store last time clock was synced
long elapsedSyncMillis = 0; // how long since the clock was synced
long syncLeft = 000000; // variable for counting down to clock sync

long currentMillis = 0; // will store millis()
long loopStart = 000000; // will store when loop started
long loopTime = 000000; // will store how long a loop takes
long syncStart = 000000; // will store when clock sync was started
long syncElapsed = 000000; // will store how long a clock sync takes
long timeSkew = 000000; // how much time differs since last clock sync in seconds
long timeNow = 000000; // will store now()

long pulsesSinceStart = 0; // various tick counters, 1 tick = 1/1000 kwh
long pulsesLastTenSeconds = 0;
long pulses = 0;
long pulsesLastHour = 0;
long pulsesLastDay = 0;
long pulsesLastWeek = 0;
long pulsesLastMonth = 0;
long pulsesLastYear = 0;

int lastTenSeconds = 0; // time shifts
int lastMinute = 0;
int lastHour = 0;
int lastDay = 0;
int lastWeek = 0;
int lastMonth = 0;
int lastYear = 0;

long lastSync = 0000000000; //time for sync, epoch()
int timeZone = 3600; //adjustment for timezone, seconds

byte mac[] = {  
  0x90, 0xA2, 0xDA, 0x0C, 0x00, 0x76 }; // MAC address, printed on a sticker on the shield
IPAddress ip(192,168,1,155); // ethernet shields wanted IP
IPAddress gateway(192,168,1,1); // internet access via router
IPAddress subnet(255,255,255,0); //subnet mask
IPAddress logServer(192,168,1,30); // http/mysql server to log to
EthernetServer server(80); // this arduinos web server port
EthernetClient client; // this arduinos web client

String readString; // string read from servers interface
bool led5 = false;
char lastChar;

// settings for timesync
unsigned int localPort = 8888; // local port to listen for UDP packets
IPAddress timeServer(192, 43, 244, 18); // time.nist.gov NTP server
const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
EthernetUDP Udp; // A UDP instance to let us send and receive packets over UDP

void setup() {
  pinMode(buttonPin2, INPUT); // initialize the pushbuttons pins as inputs
  pinMode(buttonPin3, INPUT);
  pinMode(ledPin4, OUTPUT); // initialize the LED pins as outputs
  pinMode(ledPin5, OUTPUT);
  pinMode(ledPin6, OUTPUT);
  Ethernet.begin(mac, ip, subnet, gateway); // start ethernet
  server.begin(); //start arduinos web server
  Serial.begin(9600); // start serial communication
  Udp.begin(localPort); // start UDP
}

//////////////////////////////////// main loop ////////////////////////////////////
void loop() {
  ///////////////////// check if there is a 'g' on serial input
  if (Serial.available() > 0) // check for serial input
  {
    byte inChar;
    inChar = Serial.read();
    if(inChar == 'g')
    {
      serverTest(); // test if you can GET index.html
    }
  }
  ///////////////////// webserver
  /*
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
   client.println("HTTP/1.1 204 Zoomkat");
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
   client.print(" ");
   client.print(day());
   client.print("/");
   client.print(month());
   client.print(" ");
   client.print(year());
   client.println(" (EST)<br>");
   client.print("Time for last sync was: "); // print the hour, minute and second for last sync
   client.print((lastSync  % 86400L) / 3600);
   client.print(':');  
   if ( ((lastSync % 3600) / 60) < 10 ) { // leading zero
   client.print('0');
   }
   client.print((lastSync  % 3600) / 60);
   client.print(':');
   if ( (lastSync % 60) < 10 ) { // leading zero
   client.print('0');
   }
   client.print(lastSync %60); // print the second
   client.print(" (UTC), ");
   if ( timeSkew > 1000000) { // if skew is abnormal, as in just after startup
   timeSkew = 0;
   }
   client.print("skew was: "); // print the skew at last sync
   client.print(timeSkew);
   client.println(" seconds<br>");
   //client.print("Unix time is: "); // print the function now()
   //client.print(now());
   //client.println("<br>");          
   client.print("Process have been running for: "); // print how long this have been running
   client.print(currentMillis);
   client.print(" ms, last sync was at: ");
   client.print(syncMillis);
   client.print(" ms, loop time is: ");
   client.print(loopTime);
   client.println(" ms<br>"); 
   client.print("Time since last sync: "); // print time since last sync
   client.print(elapsedSyncMillis);
   client.print(" ms, it took: ");
   client.print(syncElapsed);
   client.print(" ms, will sync at: ");
   client.print(syncInterval);
   client.print(" ms, there is: ");
   client.print(syncLeft);
   client.println(" ms left<br>");
   //client.print("Reason for sync: "); // why did we sync
   //client.print(syncReasonText[syncReason]);
   //client.println("<br>");
   
   client.print("Sensor value 0 is: "); // sensor values
   client.print(sensorValue0);
   client.println("<br>");
   client.print("Sensor value 1 is: ");
   client.print(sensorValue1);
   client.println("<br>");
   client.print("Sensor value 2 is: ");
   client.print(sensorValue2);
   client.println("<br>");
   client.print("Sensor value 3 is: ");
   client.print(sensorValue3);
   client.println("<br>");
   client.print("Sensor value 4 is: ");
   client.print(sensorValue4);
   client.println("<br>");
   client.print("Sensor value 5 is: ");
   client.print(sensorValue5);
   client.println("<br>");
   
   client.print("Pulses since start: "); // print pulses
   client.print(pulsesSinceStart);
   client.println("<br>");
   
   client.print("Pulses last ten seconds: ");
   client.print(pulsesLastTenSeconds);
   client.println("<br>");
   
   client.print("Pulses last minute: ");
   client.print(pulses);
   client.println("<br>");
   
   client.print("Pulses last hour: ");
   client.print(pulsesLastHour);
   client.println("<br>");
   client.print("Pulses last day: ");
   client.print(pulsesLastDay);
   client.println("<br>");
   client.print("Pulses last week: ");
   client.print(pulsesLastWeek);
   client.println("<br>");
   client.print("Pulses last month: ");
   client.print(pulsesLastMonth);
   client.println("<br>");
   client.print("Pulses last year: ");
   client.print(pulsesLastYear);
   client.println("<br>");
   
   client.println("LED: ");
   client.println("<a href=\"/?on\" target=\"inlineframe\">ON</a>"); 
   client.println("<a href=\"/?off\" target=\"inlineframe\">OFF</a><br>");
   client.println("<a href=\"/?tick\" target=\"inlineframe\">Send tick</a>");
   client.println("<IFRAME name=inlineframe style=\"display:none\" >");          
   client.println("</IFRAME><br>");
   client.print("LED is ");
   if (led5) {
   client.println("On");
   }
   else {
   client.println("Off");
   }
   client.println(" (Refresh page to see actual state)");
   client.println("</BODY>");
   client.println("</HTML>");
   }
   delay(1);
   client.stop(); //stopping client
   // control arduino pin
   if(readString.indexOf("on") > 0) { //checks for on
   digitalWrite(5, HIGH); // set pin 5 high
   Serial.println("Led5 turned on from webpage");
   led5 = true;
   }
   if(readString.indexOf("off") > 0) { //checks for off
   digitalWrite(5, LOW); // set pin 5 low
   Serial.println("Led5 turned off from webpage");
   led5 = false;
   }
   if(readString.indexOf("tick") > 0) { //checks for tick
   pulsesSinceStart++; // +1 on all tick counters
   pulsesLastTenSeconds++;
   pulses++;
   pulsesLastHour++;
   pulsesLastDay++;
   pulsesLastWeek++;
   pulsesLastMonth++;
   pulsesLastYear++;
   Serial.print("Registered tick from webpage at "); 
   Serial.print(hour());
   Serial.print(":");
   if ( (minute()) < 10 ) {
   Serial.print("0");
   }
   Serial.print(minute());
   Serial.print(":");
   if ( (second()) < 10 ) {
   Serial.print("0");
   }
   Serial.print(second());
   Serial.print(", pulses since start is now ");
   Serial.print(pulsesSinceStart);
   Serial.print(", ");
   Serial.print(logLeft);
   Serial.println(" ms till next log write");
   }
   readString=""; //clearing string for next read
   }
   }
   }
   }
   */
  ///////////////////// checking some times
  currentMillis = millis(); // millis right now
  loopTime = (currentMillis - loopStart); // how long does a loop on this program take
  loopStart = millis(); // now we start timing the loop
  elapsedSyncMillis = (currentMillis - syncMillis); // how much time elapsed since last clock sync
  syncLeft = (syncInterval - elapsedSyncMillis); // how long till next clock sync
  elapsedLogMillis = (currentMillis - logMillis); // how much time elapsed since last log write
  logLeft = (logInterval - elapsedLogMillis); // how long till next log write
  ///////////////////// read buttons and analog inputs
  buttonState2 = digitalRead(buttonPin2);
  buttonState3 = digitalRead(buttonPin3);
  sensorValue0 = analogRead(sensorPin0);
  sensorValue1 = analogRead(sensorPin1);
  sensorValue2 = analogRead(sensorPin2);
  sensorValue3 = analogRead(sensorPin3);
  sensorValue4 = analogRead(sensorPin4);
  sensorValue5 = analogRead(sensorPin5);
  ///////////////////// check if button2 has been pressed
  if (buttonState2 == HIGH) { // light red LED if button is pressed
    digitalWrite(ledPin6, HIGH);
  }
  else {
    digitalWrite(ledPin6, LOW);
  }
  if (buttonState2 != lastButtonState2) { // light red LED if button is pressed
    if (buttonState2 == HIGH) { // if the current state is HIGH then the button went from off to on
      pulsesSinceStart++; // +1 on all tick counters
      pulsesLastTenSeconds++;
      pulses++;
      pulsesLastHour++;
      pulsesLastDay++;
      pulsesLastWeek++;
      pulsesLastMonth++;
      pulsesLastYear++;
      Serial.print("Registered tick from button2 at "); 
      Serial.print(hour());
      Serial.print(":");
      if ( (minute()) < 10 ) {
        Serial.print("0");
      }
      Serial.print(minute());
      Serial.print(":");
      if ( (second()) < 10 ) {
        Serial.print("0");
      }
      Serial.print(second());
      Serial.print(", pulses since start is now ");
      Serial.print(pulsesSinceStart);
      Serial.print(", ");
      Serial.print(logLeft);
      Serial.println(" ms till next log write");
    }
  }
  lastButtonState2 = buttonState2; // save button2 state till next loop
  ///////////////////// check if it's time to write a log
  if ( elapsedLogMillis > logInterval) {
    logReason = 1;
    minuteLog();  // write to log
  }
  if (timeStatus() == timeNotSet) { // sync if time is not set
    syncReason = 1;
    setTime();
  }
  else {
    if (elapsedSyncMillis > syncInterval) { // sync if it is too long since last sync
      syncReason = 2;
      setTime();
    }
    else {
      if (buttonState3 == HIGH) { // sync if button on digital in 3 is pressed
        syncReason = 3;
        setTime();
      }
    }
  }
}

//////////////////////////////////// sync clock ////////////////////////////////////
void setTime() {
  syncMillis = currentMillis; // save the last time you synced time
  timeNow = now(); // time now
  syncStart = millis(); // start timing the sync    
  digitalWrite(ledPin4, HIGH); // turn LED on
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  delay(1000); // wait to see if a reply is available
  if ( Udp.parsePacket() ) { // We've received a packet, read the data from it
    Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer
    buttonState = 1;
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
  lastSync = (epoch);    
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
  adjustTime(timeZone); // adjust time to timezone
  timeSkew = (now() - timeNow - syncElapsed / 1000); // how much did time differ
  digitalWrite(ledPin4, LOW); // turn LED off
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

//////////////////////////////////// GET index.html ////////////////////////////////////
void serverTest() {
  /*
  if (client.connect(logServer, 80)) {
   Serial.println("connected");
   client.println("GET /index.html HTTP/1.0");
   client.println();
   } 
   else {
   Serial.println("connection failed");
   Serial.println();
   }
   while(client.connected() && !client.available()) delay(1); //waits for data
   while (client.connected() || client.available()) { //connected or data available
   char c = client.read();
   Serial.print(c);
   }
   
   Serial.println();
   Serial.println("Disconnecting...");
   Serial.println();
   client.stop();
   */
}

//////////////////////////////////// write to minuteLog ////////////////////////////////////
void minuteLog() {
  logMillis = currentMillis; // save the last time you wrote to log
  if (client.connect(logServer, 80)) {
    Serial.print("---Writing to ");
    Serial.print(logServer);
    Serial.println("/arduinolog/pulseLog.php");
    client.print("GET /arduinolog/pulseLog.php?");
    client.print("unixTime=");
    client.print(now());
    client.print("&&pulses=");
    client.print(pulses);
    client.print("&&currentR=");
    client.print(sensorValue0);
    client.print("&&currentS=");
    client.print(sensorValue1);
    client.print("&&currentT=");
    client.print(sensorValue2);
    client.print("&&temp1=");
    client.print(sensorValue3);
    client.print("&&temp2=");
    client.print(sensorValue4);
    client.print("&&temp3=");
    client.print(sensorValue5);
    client.print("&&event=");
    client.print(logReasonText[logReason]);
    client.println(" HTTP/1.0");
    //client.println("User-Agent: Arduino");
    //client.println("Accept: text/html");
    client.println("Connection: close");
    client.println();
    Serial.print("---Reset pulses, was ");
    Serial.println(pulses);
    pulses = 0; // reset pulses
    //lastChar = c;
  }
  else {
    Serial.print("*** Connection to ");
    Serial.print(logServer);
    Serial.println(" failed");
  }
  while(client.connected() && !client.available()) delay(1); //waits for data
  while (client.connected() || client.available()) { //connected or data available
    char c = client.read();

    Serial.print(c);
  }

  Serial.println();
  Serial.print("---Disconnecting from ");
  Serial.println(logServer);
  client.stop();
}




















