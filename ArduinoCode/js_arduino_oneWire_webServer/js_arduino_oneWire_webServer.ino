#include <OneWire.h> // oneWire libraries
#include <DallasTemperature.h>
#include <SPI.h> // ethernet libraries
#include <Ethernet.h>
#include <EthernetServer.h>

// Data wire is plugged into pin 2 on the Arduino (can be any digital I/O pin)
#define ONE_WIRE_BUS 3 // oneWire bus bin

// oneWire settings
OneWire oneWire(ONE_WIRE_BUS); // setup oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire); // pass our oneWire reference to Dallas Temperature
float tempValue[] = {
  0, 1};

// IP settings
byte mac[] = {  
  0x90, 0xA2, 0xDA, 0x0C, 0x00, 0x76 }; // MAC address, printed on a sticker on the shield
//IPAddress ip(192,168,1,155); // ethernet shields wanted IP
//IPAddress gateway(192,168,1,1); // internet access via router
//IPAddress subnet(255,255,255,0); //subnet mask
EthernetServer server(80); // this arduinos web server port
String readString; // string read from servers interface

int numberOfSensors; // will store how many sensors we have
byte i; // will hold value for different loops
boolean mainLoop = false;

//////////////////////////////////// setup ////////////////////////////////////
void setup(void)
{
  // start serial port
  Serial.begin(9600);
  Serial.println("Started serial communication");
  Serial.println("Starting 1-wire sensors...");
  sensors.begin(); // start up the oneWire library
  delay(5000);  //important on linux as serial port can lock up otherwise
  Serial.println("Discovering and counting sensors...");
  numberOfSensors = discoverOneWireDevices(); // count sensors
  Serial.println("Starting ethernet...");
  //Ethernet.begin(mac, ip, subnet, gateway); // start ethernet
  Ethernet.begin(mac); // start ethernet
  Serial.println("Starting web server...");
  server.begin(); //start arduinos web server
}

//////////////////////////////////// main loop ////////////////////////////////////
void loop(void) {
  if (!mainLoop) {
    Serial.println("Main loop started");
    mainLoop = true;
  }
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
            for(i=0; i < numberOfSensors; i++) { // print temperatures
              client.print("temp");
              client.print(i);
              client.print(": ");
              client.print(tempValue[i]);
              client.println("<br>");
            }
            client.println("<br>");
            client.println("<a href=\"/?pulse\" target=\"inlineframe\">Send pulse</a><br>");
            client.println("<IFRAME name=inlineframe style=\"display:none\" >");          
            client.println("</IFRAME><br>");
            client.println("</BODY>");
            client.println("</HTML>");
          }
          delay(1);
          client.stop(); //stopping client
          if(readString.indexOf("readTemps") > 0) { //will read 1-wire temps
            getTemperatures();
          }
        }
        readString=""; //clearing string for next read
      }
    }
  }
}

/////////////////////////////// request temperatures ///////////////////////////////
void getTemperatures(void) {
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  for(i=0; i < numberOfSensors; i++) { // read each of our sensors and print the value
    Serial.print("Temperature for Device ");
    Serial.print( i );
    Serial.print(" is: ");
    //Serial.println( sensors.getTempCByIndex(i) ); // 0 refers to the first IC on the wire
    tempValue[i] = sensors.getTempCByIndex(i);
    Serial.println(tempValue[i]);
  }
  Serial.println();
}

/////////////////////////////// discover sensors ////////////////////////////////
int discoverOneWireDevices(void) {
  byte present = 0;
  byte data[12];
  byte addr[8];
  int count = 0;
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
  Serial.println("That's it.");
  oneWire.reset_search();
  return count;
}






