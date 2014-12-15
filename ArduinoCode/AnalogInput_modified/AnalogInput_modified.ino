/*
 
 */

  // declare strings:
byte stringCounter;

  // declare constants:
int sensorPin = A0;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);
  //stringCounter = String ();
     // initialize serial communications at 9600 bps:
  Serial.begin(9600);

}

void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);    
  // turn the ledPin on
  digitalWrite(ledPin, HIGH);  
  // stop the program for <sensorValue> milliseconds:
  delay(sensorValue);          
  // turn the ledPin off:        
  digitalWrite(ledPin, LOW);
  stringCounter = stringCounter + 1;
  // print the results to the serial monitor:
  Serial.print("counts =  ");
  Serial.print(stringCounter);
  Serial.print("\tsensor = ");
  Serial.println(sensorValue);
  // stop the program for for <sensorValue> milliseconds:
  delay(sensorValue);
}
