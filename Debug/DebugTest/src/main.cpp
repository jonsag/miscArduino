#include <Arduino.h>

#define __DEBUG__ // uncomment to turn on debugging messages

#ifdef __DEBUG__
#define DEBUG(...) printf(__VA_ARGS__)
#define DEBUGLN(...) printf("\n")
#define DEBUGDLN(...) printf("\n\n")
#else
#define DEBUG(...)
#define DEBUGLN(...)
#define DEBUGDLN(...)
#endif

int i = 0;

void setup() {
  Serial.begin(9600);

  printf("--- Serial debug example ---\n");

  DEBUG("Debugging is ON\n");
}

void loop() {
  DEBUG("Debug line:\n");
  DEBUG(i);
  DEBUGLN();
  delay(500);

  DEBUG("Double line feed");
  DEBUGDLN();

  i++;
}