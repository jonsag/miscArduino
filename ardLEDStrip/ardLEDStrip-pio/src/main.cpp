#include <Arduino.h>

// A non-blocking everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#include <config.h>
#include <leds.h>

//#include <demo.h>

#include <button.h>

#include <test.h>

// setup() function -- runs once at startup --------------------------------
void setup()
{
  /***********
   * Serial
   ***********/
  if (DEBUG || INFO)
  {
    Serial.begin(serialBaudRate); // serial baudrate
  }

  /*******************************
   * Print start information
   *******************************/
  infoMessln();
  infoMessln(programName); // print information
  infoMessln(date);
  infoMess("by ");
  infoMessln(author);
  infoMessln(email);
  infoMessln();

  /*******************************
   * Debugging
   *******************************/
  if (!DEBUG && INFO)
  {
    infoMessln("Debug is off");
    infoMessln();
  }

  /*******************************
   * Button
   *******************************/
  // link functions to be called on events.
  btn.attachClick(HandleClick);
  btn.attachLongPressStart(HandleLongPress);
  btn.attachDoubleClick(HandleDoubleClick);

  btn.setLongPressIntervalMs(LongPressInterval);

  /*******************************
   * NeoPixel Strip
   *******************************/
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  strip.begin();                   // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();                    // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS
}

// loop() function -- runs repeatedly as long as board is on ---------------
void loop()
{
  //RunDemo();

  /*
  colorWipe(strip.Color(255,   0,   0)     , 50); // Red
  colorWipe(strip.Color(  0, 255,   0)     , 50); // Green
  colorWipe(strip.Color(  0,   0, 255)     , 50); // Blue
  colorWipe(strip.Color(  255   ,   255,   255), 50); // True white (not RGB white)
  */

  //test();
 
  btn.tick();
}