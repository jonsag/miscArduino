String programName = "ardLEDStrip";
String date = "20250628";
String author = "Jon Sagebrand";
String email = "jonsagebrand@gmail.com";

/*******************************
  Debugging
*******************************/
#define DEBUG 0 // debugMess is off when 0
#define INFO 1

#if DEBUG
#define debugMess(x) Serial.print(x)
#define debugMessln(x) Serial.println(x)
#else
#define debugMess(x)
#define debugMessln(x)
#endif

#if INFO || DEBUG
#define infoMess(x) Serial.print(x)
#define infoMessln(x) Serial.println(x)
#else
#define infoMess(x)
#define infoMessln(x)
#endif

/***********
 * Serial
 ***********/
#define serialBaudRate 9600



/***********
 * Button
 ***********/
#define ButtonPin 2
#define LongPressInterval 500

int status = 0;


/***********
 * NeoPixel Strip
 ***********/
// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#ifdef ESP32
// Cannot use 6 as output for ESP. Pins 6-11 are connected to SPI flash. Use 16 instead.
#define LED_PIN 16
#else
#define LED_PIN 7
#endif

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 44
#define BRIGHTNESS 50 // Set BRIGHTNESS to about 1/5 (max = 255)

//#define WAIT 50

const int WAIT = 50;

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

unsigned long pixelPrevious = 0;   // Previous Pixel Millis
unsigned long patternPrevious = 0; // Previous Pattern Millis
int patternCurrent = 0;            // Current Pattern Number
#define patternInterval 5000       // Pattern Interval (ms)
bool patternComplete = false;

int pixelInterval = 50;           // Pixel Interval (ms)
int pixelQueue = 0;               // Pattern Pixel Queue
int pixelCycle = 0;               // Pattern Pixel Cycle
uint16_t pixelNumber = LED_COUNT; // Total Number of Pixels
