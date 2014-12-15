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

EthernetServer webServer(80);

void setup()
{
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }

  webServer.begin();

}

void loop()
{
  server();
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

          webClient.println("<html><body><p>Testing...</p></body></html>");

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


