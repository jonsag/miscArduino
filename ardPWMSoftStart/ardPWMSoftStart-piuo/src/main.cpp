#include <Arduino.h>

const int motorPin = 3;     // PWM capable pin for motor control
const int SpeedPotPin = A0; // Analog pin for potentiometer (speed control)
const int RampPotPin = A1;  // Analog pin for potentiometer (ramp control)

int SpeedValue = 0;
int RampValue = 0;

float motorSpeed = 0;
float Ramp = 0;

void setup()
{
  pinMode(motorPin, OUTPUT);
  Serial.begin(9600);
}

void loop()
{
  SpeedValue = analogRead(SpeedPotPin); // Read potentiometer value (0-1023)
  RampValue = analogRead(RampPotPin);   // Read potentiometer value (0-1023)

  motorSpeed = map(SpeedValue, 0, 1023, 0, 255); // Map to PWM range (0-255)
  Ramp = map(RampValue, 0, 1023, 0, 255);        // Map to PWM range (0-255)

  analogWrite(motorPin, motorSpeed); // Set motor speed using PWM
  Serial.print("Speed Value: ");
  Serial.print(SpeedValue);

  Serial.print("\tMotor Speed: ");
  Serial.print(motorSpeed / 255 * 100);

  Serial.print("\tRamp Value: ");
  Serial.print(RampValue);

  Serial.print("%\t\tRamp: ");
  Serial.print(Ramp / 255 * 100);
  Serial.println("%");
  delay(10);
}
