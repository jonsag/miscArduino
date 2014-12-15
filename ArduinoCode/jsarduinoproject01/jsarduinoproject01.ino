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
const int buttonPin2 = 2;    // simulate ticks button
const int buttonPin3 = 3;    // the number of the pushbutton pin to sync clock
const int ledPin4 =  4;      // the number of the yellow LED pin
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
int ledState6 = LOW;

int sensorValue0 = 0;
int sensorValue1 = 0;
int sensorValue2 = 0;
int sensorValue3 = 0;
int sensorValue4 = 0;
int sensorValue5 = 0;

// reason for syncing clock
int reason = 0;
char* syncReason[] = {
  "initializing", "time not set", "sync time was achieved", "button was pressed"};

// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval = 10000;           // interval at which to blink (milliseconds)
long previousMillis = 0;        // will store last time LED was updated
long currentMillis = 0;
long elapsedMillis = 0;
long loopTime = 000000;
long loopStart = 000000;
long syncStart = 000000;
long syncElapsed = 000000;
long timeSkew = 000000;
long timeNow = 000000;

/* how often will we sync the clock
 1 min = 60000
 1 hour = 3600000
 1 day = 86400000
 1 week = 604800000 */
long syncMillis = 3600000;

// variable for counting down to sync
long syncLeft = 000000;

// various tick counters, 1 tick = 1/1000 kwh
long ticksSinceStart = 0;
long ticksLastTenSeconds = 0;
long ticksLastMinute = 0;
long ticksLastHour = 0;
long ticksLastDay = 0;
long ticksLastWeek = 0;
long ticksLastMonth = 0;
long ticksLastYear = 0;

// time shifts
int lastTenSeconds = 0;
int lastMinute = 0;
int lastHour = 0;
int lastDay = 0;
int lastWeek = 0;
int lastMonth = 0;
int lastYear = 0;

//time for sync
long lastSync = 0000000000;
//adjustment for timezone, seconds
int timeZone = 3600;

// MAC address, printed on a sticker on the shield
byte mac[] = {  
  0x90, 0xA2, 0xDA, 0x0C, 0x00, 0x76 };
byte ip[] = {
  192, 168, 1, 155};

// ethernet server
EthernetServer webServer(80);

// settings for server to GET
IPAddress logServer(192, 168, 1, 30);
EthernetClient serverClient;
bool connected = false;

// settings for timesync
unsigned int localPort = 8888; // local port to listen for UDP packets
IPAddress timeServer(192, 43, 244, 18); // time.nist.gov NTP server
const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
EthernetUDP Udp; // A UDP instance to let us send and receive packets over UDP

void setup()
{
  Serial.begin(9600);

  // start Ethernet
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
  // give the Ethernet shield some to initialize:
  delay(500);
  Serial.println("Initializing ethernet...");

  // start UDP
  Udp.begin(localPort);

  // start the webserver
  Serial.println("Server enabled, starting...");
  webServer.begin();  

  // initialize the pushbuttons pins as inputs
  pinMode(buttonPin2, INPUT);
  pinMode(buttonPin3, INPUT);
  // initialize the LED pins as output
  pinMode(ledPin4, OUTPUT);
  pinMode(ledPin6, OUTPUT);
}

void loop()
{
  currentMillis = millis();

  // how long does a loop on this program take
  loopTime = (currentMillis - loopStart);
  loopStart = millis();
  elapsedMillis = (currentMillis - previousMillis);

  // read buttons
  buttonState2 = digitalRead(buttonPin2);
  buttonState3 = digitalRead(buttonPin3);

  // light red LED if button is pressed
  if (buttonState2 == HIGH) {
    digitalWrite(ledPin6, HIGH);
  }
  else {
    digitalWrite(ledPin6, LOW);
  }

  // compare the buttonState to its previous state
  if (buttonState2 != lastButtonState2) {
    // if the state has changed, increment the counter
    if (buttonState2 == HIGH) {
      // if the current state is HIGH then the button
      // went from off to on:
      ticksSinceStart++;
      ticksLastTenSeconds++;
      ticksLastMinute++;
      ticksLastHour++;
      ticksLastDay++;
      ticksLastWeek++;
      ticksLastMonth++;
      ticksLastYear++;
      Serial.print("Registered tick at "); 
      Serial.print(hour());
      Serial.print(":");
      if ( (minute()) < 10 ) {
        // In the first 10 minutes of each hour, we'll want a leading '0'
        Serial.print("0");
      }
      Serial.print(minute());
      Serial.print(":");
      if ( (second()) < 10 ) {
        // In the first 10 minutes of each hour, we'll want a leading '0'
        Serial.print("0");
      }
      Serial.print(second());
      Serial.print(" , ticks since start is now ");
      Serial.println(ticksSinceStart);
    }
  }
  lastButtonState2 = buttonState2;

  // reset counters on certain times
  // reset LastTenSeconds
  // Serial.println("Reset ticksLastTenSeconds");
  // reset LastMinute
  if (minute() != lastMinute) {
    Serial.print("Time is now ");
    Serial.print(hour());
    Serial.print(":");
    if ( (minute()) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print("0");
    }
    Serial.print(minute());
    Serial.print(":");
    if ( (second()) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print("0");
    }
    Serial.println(second());
    minuteLog();
  }
  lastMinute = minute();
  // reset LastHour
  if (hour() != lastHour) {
    ticksLastHour = 0;
    Serial.println("Reset ticksLastHour");
  }
  lastHour = hour();
  // reset LastDay
  if (day() != lastDay) {
    ticksLastDay = 0;
    Serial.println("Reset ticksLastDay");
  }
  lastDay = day();
  // reset LastWeek
  if (weekday() == 2) {
    if (lastWeek == 1) {
      ticksLastWeek = 0;
      Serial.println("Reset ticksLastWeek");
    }
  }
  lastWeek = weekday();
  // reset LastMonth
  if (month() != lastMonth) {
    ticksLastMonth = 0;
    Serial.println("Reset ticksLastMonth");
  }
  lastMonth = month();
  // reset LastYear
  if (year() != lastYear) {
    ticksLastYear = 0;
    Serial.println("Reset ticksLastYear");
  }
  lastYear = year();

  // read sensor values
  sensorValue0 = analogRead(sensorPin0);
  sensorValue1 = analogRead(sensorPin1);
  sensorValue2 = analogRead(sensorPin2);
  sensorValue3 = analogRead(sensorPin3);
  sensorValue4 = analogRead(sensorPin4);
  sensorValue5 = analogRead(sensorPin5);

  // sync if time is not set
  if (timeStatus() == timeNotSet) {
    reason = 1;
    setTime();
  }
  else {
    // sync if it is too long since last sync
    if (elapsedMillis > syncMillis) {
      reason = 2;
      setTime();
    }
    else {
      // sync if button on digital in 3 is pressed
      if (buttonState3 == HIGH) {
        reason = 3;
        setTime();
      }
      else {
        //        showTime();
        server();
        readServerClient();
      }
    }
  }
}


void setTime()
{

  // save the last time you blinked the LED
  previousMillis = currentMillis;

  // time now
  timeNow = now();

  // start timing the sync
  syncStart = millis();

  // turn LED on:    
  digitalWrite(ledPin4, HIGH);  

  sendNTPpacket(timeServer); // send an NTP packet to a time server

    // wait to see if a reply is available
  delay(1000);  

  if ( Udp.parsePacket() ) {  
    // We've received a packet, read the data from it
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

  // now convert NTP time into everyday time:
  Serial.print("Unix time: ");
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;    
  // subtract seventy years:
  unsigned long epoch = secsSince1900 - seventyYears;  
  // print Unix time:
  Serial.print(epoch);
  Serial.print("\t");

  lastSync = (epoch);    

  // print the hour, minute and second:
  Serial.print("UTC time: "); // UTC is the time at Greenwich Meridian (GMT)
  Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
  Serial.print(':');  
  if ( ((epoch % 3600) / 60) < 10 ) {
    // In the first 10 minutes of each hour, we'll want a leading '0'
    Serial.print('0');
  }
  Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
  Serial.print(':');
  if ( (epoch % 60) < 10 ) {
    // In the first 10 seconds of each minute, we'll want a leading '0'
    Serial.print('0');
  }
  Serial.println(epoch %60); // print the second

  // write to eventLog
  eventLog();

  // stop timing of sync
  syncElapsed = (millis() - syncStart);

  setTime(epoch);
  adjustTime(timeZone);
  timeSkew = (now() - timeNow - syncElapsed / 1000);

  // turn LED off:
  digitalWrite(ledPin4, LOW);

}

void showTime()
{

  // digital clock display of the time
  Serial.print("Current time is: ");
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.print(" (EST)");

  //Serial.println(lastSync);

  // print the hour, minute and second:
  Serial.print("\t Time for last sync was: ");
  Serial.print((lastSync  % 86400L) / 3600);
  Serial.print(':');  
  if ( ((lastSync % 3600) / 60) < 10 ) {
    // In the first 10 minutes of each hour, we'll want a leading '0'
    Serial.print('0');
  }
  Serial.print((lastSync  % 3600) / 60);
  Serial.print(':');
  if ( (lastSync % 60) < 10 ) {
    // In the first 10 seconds of each minute, we'll want a leading '0'
    Serial.print('0');
  }
  Serial.print(lastSync %60); // print the second
  Serial.println(" (UTC)");

}

void server()
{

  // listen for incoming clients
  EthernetClient webClient = webServer.available();
  if (webClient) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (webClient.connected()) {
      if (webClient.available()) {
        char c = webClient.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          webClient.println("HTTP/1.1 200 OK");
          webClient.println("Content-Type: text/html");
          webClient.println();

          // digital clock display of the current time
          webClient.print("Current time is: ");
          webClient.print(hour());
          webClient.print(":");
          if ( (minute()) < 10 ) {
            // In the first 10 minutes of each hour, we'll want a leading '0'
            webClient.print("0");
          }
          webClient.print(minute());
          webClient.print(":");
          if ( (second()) < 10 ) {
            // In the first 10 minutes of each hour, we'll want a leading '0'
            webClient.print("0");
          }
          webClient.print(second());
          webClient.print(" ");
          webClient.print(day());
          webClient.print("/");
          webClient.print(month());
          webClient.print(" ");
          webClient.print(year());
          webClient.print(" (EST)");
          webClient.println("<br />");

          // print the hour, minute and second for last sync:
          webClient.print("Time for last sync was: ");
          webClient.print((lastSync  % 86400L) / 3600);
          webClient.print(':');  
          if ( ((lastSync % 3600) / 60) < 10 ) {
            // In the first 10 minutes of each hour, we'll want a leading '0'
            webClient.print('0');
          }
          webClient.print((lastSync  % 3600) / 60);
          webClient.print(':');
          if ( (lastSync % 60) < 10 ) {
            // In the first 10 seconds of each minute, we'll want a leading '0'
            webClient.print('0');
          }
          webClient.print(lastSync %60); // print the second
          webClient.print(" (UTC)");
          webClient.println("<br />");

          if ( timeSkew > 1000000) {
            timeSkew = 0;
          }

          // print the skew at last sync
          webClient.print("At last sync the skew was: ");
          webClient.print(timeSkew);
          webClient.print(" seconds");
          webClient.println("<br />");

          // print the function now()
          webClient.print("Unix time is: ");
          webClient.print(now());
          webClient.println("<br />");          

          // print how long this have been running
          webClient.print("Process have been running for: ");
          webClient.print(currentMillis);
          webClient.print(" ms, last sync was at: ");
          webClient.print(previousMillis);
          webClient.print(" ms, loop time is: ");
          webClient.print(loopTime);
          webClient.print(" ms");
          webClient.println("<br />");

          syncLeft = (syncMillis - elapsedMillis);

          // print time since last sync 
          webClient.print("Time since last sync: ");
          webClient.print(elapsedMillis);
          webClient.print(" ms, it took: ");
          webClient.print(syncElapsed);
          webClient.print(" ms, will sync at: ");
          webClient.print(syncMillis);
          webClient.print(" ms, there is: ");
          webClient.print(syncLeft);
          webClient.print(" ms left");
          webClient.println("<br />");

          // why did we sync
          webClient.print("Reason for sync: ");
          webClient.print(syncReason[reason]);
          webClient.println("<br />");

          // analog input 0
          webClient.print("Analog input A0 is: ");
          webClient.print(sensorValue0);
          webClient.println("<br />");
          // analog input 1
          webClient.print("Analog input A1 is: ");
          webClient.print(sensorValue1);
          webClient.println("<br />");
          // analog input 2
          webClient.print("Analog input A2 is: ");
          webClient.print(sensorValue2);
          webClient.println("<br />");
          // analog input 3
          webClient.print("Analog input A3 is: ");
          webClient.print(sensorValue3);
          webClient.println("<br />");
          // analog input 4
          webClient.print("Analog input A4 is: ");
          webClient.print(sensorValue4);
          webClient.println("<br />");
          // analog input 5
          webClient.print("Analog input A5 is: ");
          webClient.print(sensorValue5);
          webClient.println("<br />");

          // print ticks
          webClient.print("Ticks since start: ");
          webClient.print(ticksSinceStart);
          webClient.println("<br />");
          webClient.print("Ticks last ten seconds: ");
          webClient.print(ticksLastTenSeconds);
          webClient.println("<br />");
          webClient.print("Ticks last minute: ");
          webClient.print(ticksLastMinute);
          webClient.println("<br />");
          webClient.print("Ticks last hour: ");
          webClient.print(ticksLastHour);
          webClient.println("<br />");
          webClient.print("Ticks last day: ");
          webClient.print(ticksLastDay);
          webClient.println("<br />");
          webClient.print("Ticks last week: ");
          webClient.print(ticksLastWeek);
          webClient.println("<br />");
          webClient.print("Ticks last month: ");
          webClient.print(ticksLastMonth);
          webClient.println("<br />");
          webClient.print("Ticks last year: ");
          webClient.print(ticksLastYear);
          webClient.println("<br />");

          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    webClient.stop();
  }
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:         
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket();
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void minuteLog() {
  if (!connected) {
    Serial.println("Not connected to logServer");

    if (serverClient.connect(logServer, 80)) {
      connected = true;

      Serial.print("---Writing to ");
      Serial.print(logServer);
      Serial.println("/arduinolog/minuteLog.php");

      /*
    serverClient.print("GET /arduinolog/minuteLog.php?");
       serverClient.print("unixTime=");
       serverClient.print(now());
       serverClient.print("&&minuteTicks=");
       serverClient.print(ticksLastMinute);
       serverClient.print("&&currentR=");
       serverClient.print(sensorValue0);
       serverClient.print("&&currentS=");
       serverClient.print(sensorValue1);
       serverClient.print("&&currentT=");
       serverClient.print(sensorValue2);
       serverClient.print("&&temp1=");
       serverClient.print(sensorValue3);
       serverClient.print("&&temp2=");
       serverClient.print(sensorValue4);
       serverClient.print("&&temp3=");
       serverClient.print(sensorValue5);
       serverClient.println(" HTTP/1.1");
       serverClient.println("User-Agent: Arduino");
       serverClient.println("Accept: text/html");
       serverClient.println("Connection: close");
       serverClient.println();
       
       // if successful reset ticksLastMinute
       ticksLastMinute = 0;
       Serial.println("Reset ticksLastMinute");
       */
    }
    else {
      Serial.print("*** Connection to ");
      Serial.print(logServer);
      Serial.println(" failed");
    }
  }
  else {
    delay(500);
    while (serverClient.connected() && serverClient.available()) {
      char c = serverClient.read();
      Serial.print(c);
    }
    serverClient.stop();
    connected = false; 
  }   
}

void eventLog() {
  Serial.print("---Writing to ");
  Serial.print(logServer);
  Serial.println("/arduinolog/eventLog.php");
  /*
    serverClient.print("GET /arduinolog/eventLog.php?");
   serverClient.print("event=Synced time");
   serverClient.println(" HTTP/1.1");
   serverClient.println("User-Agent: Arduino");
   serverClient.println("Accept: text/html");
   serverClient.println("Connection: close");
   serverClient.println();
   */
}

void readServerClient() {
  /*
  if (serverClient.available()) {
   char c = serverClient.read();
   Serial.print(c);
   if (!serverClient.connected()) {
   Serial.println("Finished writing eventLog, disconnecting...");
   serverClient.stop();
   }
   }
   */
}






















