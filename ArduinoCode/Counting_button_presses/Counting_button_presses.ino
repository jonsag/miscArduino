/*
 
 */

  // this constant won't change:
const int sensorPin = A0;    // select the input pin for the potentiometer
const int ledPinRed = 6;      // select the pin for the  red LED
const int ledPinGreen = 5;      // select the pin for the  green LED
const int ledPinYellow = 4;      // select the pin for the  yellow/white LED
const int buttonPin = 3;    // the pin that the pushbutton is attached to

// Variables will change:
int Counter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button
int sensorValue = 0;  // variable to store the value coming from the sensor

void setup() {
  // declare the ledPins as an OUTPUTs:
  pinMode(ledPinRed, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinYellow, OUTPUT);
  // initialize the button pin as a input:
  pinMode(buttonPin, INPUT);
  //stringCounter = String ();
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);

}

void loop() {
  // read the pushbutton input pin:
  
  buttonState = digitalRead(buttonPin);
  
    // read the pushbutton input pin:
  buttonState = digitalRead(buttonPin);

  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      // if the current state is HIGH then the button
      // wend from off to on:
      Counter++;
      //Serial.println("on");
      Serial.print("number of pulses = \t");
      Serial.println(Counter);
    } 
    else {
      // if the current state is LOW then the button
      // wend from on to off:
      //Serial.println("off"); 
    }
  }
  // save the current state as the last state, 
  //for next time through the loop
  lastButtonState = buttonState;
}

