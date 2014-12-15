#include <SPI.h>
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <util.h>

byte mac[] = {  
  0x90, 0xA2, 0xDA, 0x0C, 0x00, 0x76 };

// server part
EthernetServer webServer(80);

// client part
IPAddress logServer(192, 168, 1, 30);
EthernetClient serverClient;

void setup()
{
  Serial.begin(9600);
  
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
  // start webServer
  webServer.begin();

  delay(1000);
  Serial.println("connecting...");

  // connect to logServer
  if (serverClient.connect(logServer, 80)) {
    Serial.println("connected");
    serverClient.println("GET /index.html HTTP/1.0");
    serverClient.println();
  }
  else {
    Serial.println("connection failed");
  }
}

void loop()
{
  server();
  client();
}


void server()
{
  EthernetClient webClient = webServer.available();

  if (webClient) {
    boolean currentLineIsBlank = true;
    while (webClient.connected()) {
      if (webClient.available()) {
        char c = webClient.read();
        if (c == '\n' && currentLineIsBlank) {
          webClient.println("HTTP/1.1 200 OK");
          webClient.println("Content-Type: text/html");
          webClient.println();

          webClient.println("<html><body><p>This is arduino webServer...</p></body></html>");

          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    webClient.stop();
  }
}

void client()
{
  if (serverClient.available()) {
    char c = serverClient.read();
    Serial.print(c);
  }
  if (!serverClient.connected()) {
   Serial.println();
   Serial.println("disconnecting.");
   serverClient.stop();
//   for(;;)
//   ;
   }
}


