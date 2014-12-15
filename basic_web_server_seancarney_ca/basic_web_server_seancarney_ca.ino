//
// Basic Web Server
// http://www.seancarney.ca/projects/basic-arduino-web-server
// Sean Carney - 2010
//

// This lets the system know we want to use the Ethernet module
// and need some additional functionality
#include <Ethernet.h>

// This is the hardware address of the Arduino on your network.
// You won't need to change this. Learn more here.
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// This is the tcp/ip address of the Arduino on your network.
// You will access the Arduino by typing it in a web browser like
// this: http://192.168.1.177 Learn more here.
// You might want to change this to be the same as your computer's
// IP address except for the last number. Learn more here.
byte ip[] = { 
  192, 168, 1, 177 };

// Create a server named webserver listening on port 80 (http). You
// don't want to change the port number. Learn more about ports or
// the server class.
Server webserver(80);

// Functions that are run once when the Arduino is started
void setup()
{

  // Bring the Arduino on the network using the MAC and IP we
  //specified before. Learn more about Ethernet.begin().
  Ethernet.begin(mac, ip);
  // Start a server on the port we specified before. Learn more
  // about server.begin().
  webserver.begin();

}

// Repeat these functions as long as the Arduino is powered on
void loop()
{

  // Create a client named webclient listening for connections to the
  // server. Learn more about Client or server.available()
  Client webclient = server.available();

  // If someone connects to the server...
  if (webclient) {

    // Create a variable to hold whether or not we have received a blank
    // line from the web browser
    boolean current_line_is_blank = true;

    // Run the following code as long as the client remains connected
    // Learn more about client.connected()
    while (webclient.connected()) {

      // If the client has sent us some data...
      // Learn more about client.available()
      if (webclient.available()) {

        // Keep the last letter of whatever they sent us
        // Learn more about client.read()
        char c = webclient.read();

        // If we've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so we can send a reply
        if (c == 'n' && current_line_is_blank) {

          // Send a basic HTTP response header (the blank line at the end
          // is required). Learn more about client.println()
          webclient.println("HTTP/1.1 200 OK");
          webclient.println("Content-Type: text/html");
          webclient.println();

          // Now send whatever html you want to appear on the web page
          webclient.println("<html><body><p>Testing...</p></body></html>");

          break;

        }

        // If we received a new line character from the client ...
        if (c == 'n') {

          // Track that the line is blank
          current_line_is_blank = true;

          // If we don't receive a new line character and don't receive an r
        } 
        else if (c != 'r') {

          // Track that we we got some data
          current_line_is_blank = false;

        }

      }

    }

    // Give the client some time to receive the page
    // Learn more about delay()
    delay(1);

    // Close the connection to the client now that we are finished
    // Learn more about client.stop()
    webclient.stop();

  }

}

