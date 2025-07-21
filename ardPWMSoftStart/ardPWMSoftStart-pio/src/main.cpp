#include <Arduino.h>

#include "OneButton.h"

const int motorPin = 3; // PWM capable pin for motor control

const int StartButtonPin = 2;

const int SpeedPotPin = A0; // Analog pin for potentiometer (speed control)
const int RampPotPin = A1;  // Analog pin for potentiometer (ramp control)

int SpeedValue = 0;
int RampValue = 0;

float motorSpeed = 0;
float MaxMotorSpeed;
float Ramp = 0;

bool StartButtonStatus = 0;

bool StatusRunning = 0;
bool StatusStarting = 0;
bool StatusRamping = 0;

const int RampMin = 1000;
const int RampMax = 10000;

long CurrentMillis;
long StartMillis;

// OneButton button(StartButtonPin, false);
OneButton button;

/*class Button
{
private:
  OneButton button;
  int value;

public:
  explicit Button(uint8_t pin) : button(pin)
  {
    button.attachClick([](void *scope)
                       { ((Button *)scope)->Clicked(); }, this);
    button.attachDoubleClick([](void *scope)
                             { ((Button *)scope)->DoubleClicked(); }, this);
    button.attachLongPressStart([](void *scope)
                                { ((Button *)s// This function will be called often, while the button1 is pressed for a long time.
cope)->LongPressed(); }, this);
  }
  Serial.print("\t\tRamp: ");

  void Clicked()
  {
    Serial.println("Click then value++");
    value++;
  }

  void DoubleClicked()
  {

    Serial.println("DoubleClick");// This function will be called often, while the button1 is pressed for a long time.

  }

  void LongPressed()
  {
    Serial.println("LongPress and the value is");
    Serial.println(value);
  }

  void handle()
  {
    button.tick();
  }
};// This function will be called often, while the button1 is pressed for a long time.


Button button(StartButtonPin);*/

/*void LongPressStart(void *oneButton)
{
  Serial.print(((OneButton *)oneButton)->getPressedMs());
  Serial.println("\t - LongPressStart()");
}

// this function will be called when the button is released.
void LongPressStop(void *oneButton)
{
  Serial.print(((OneButton *)oneButton)->getPressedMs());
  Serial.println("\t - LongPressStop()\n");
}

// this function will be called when the button is held down.
void DuringLongPress(void *oneButton)
{
  Serial.print(((OneButton *)oneButton)->getPressedMs());
  Serial.println("\t - DuringLongPress()");
}*/

void longPressStart()
{
  Serial.println("LongPress start");
} // longPressStart

void longPress()
{
  Serial.println("LongPress...");
} // longPress

void longPressStop()
{
  Serial.println("LongPress stop");
} // longPressStop

void click()
{
  Serial.println("Click.");
  if (StatusRunning)
  {
    Serial.println("Stopping...");
    StatusRunning = 0;
    motorSpeed = 0;
  }
} // click

void doubleClick()
{
  Serial.println("Double Click");
  if (!StatusRunning)
  {
    Serial.println("Starting...");
    StatusRunning = 1;
    StatusRamping = 1;

    StartMillis = CurrentMillis;
  }
} // doubleClick

void setup()
{
  Serial.begin(9600);

  pinMode(motorPin, OUTPUT);

  button.setup(StartButtonPin, false, false);

  button.attachLongPressStart(longPressStart);
  button.attachDuringLongPress(longPress);
  button.attachLongPressStop(longPressStop);
  button.setLongPressIntervalMs(1000);
  button.attachClick(click);
  button.attachDoubleClick(doubleClick);
}

void loop()
{
  CurrentMillis = millis();

  button.tick();
  // button.handle();

  if (!StatusRunning)
  {
    SpeedValue = analogRead(SpeedPotPin); // Read potentiometer value (0-1023)
    RampValue = analogRead(RampPotPin);   // Read potentiometer value (0-1023)

    MaxMotorSpeed = map(SpeedValue, 0, 1023, 0, 255); // Map to PWM range (0-255)
    Ramp = map(RampValue, 0, 1023, RampMin, RampMax); // Map to PWM range (0-255)
  }

  if (StatusRunning)
  {
    if (StatusRamping && CurrentMillis - StartMillis <= Ramp)
    {
      Serial.print("Ramping up...\t");
      Serial.print((Ramp - (CurrentMillis - StartMillis)) / 1000);
      Serial.print("\t");
      Serial.print((CurrentMillis - StartMillis) / Ramp * 100);
      Serial.print("%");

      motorSpeed = MaxMotorSpeed * ((CurrentMillis - StartMillis) / Ramp);
      Serial.print("\tMotor Speed: ");
      Serial.println(motorSpeed);
    }
    else if (StatusRamping)
    {
      Serial.println("Ramp Finished!");
      StatusRamping = 0;
      motorSpeed = MaxMotorSpeed;
    }
    analogWrite(motorPin, motorSpeed); // Set motor speed using PWM
  }
  else
  {
    analogWrite(motorPin, 0);
  }

  /*if (!StatusRunning)
  {
    Serial.print("Speed Value: ");
    Serial.print(SpeedValue);

    Serial.print("\tMotor Speed: ");
    Serial.print(motorSpeed / 255 * 100);

    Serial.print("\tRamp Value: ");
    Serial.print(RampValue);

    Serial.print("\t\tRamp: ");
    Serial.print(Ramp / RampMax * 100);
    Serial.print("%");

    Serial.print("\t\tRamp Time: ");
    Serial.print(Ramp / 1000);
    Serial.println("s");
  }*/
}