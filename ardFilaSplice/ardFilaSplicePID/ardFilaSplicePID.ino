/*
  String programName = "ardFilaSplice";
  String date = "20201226";
  String author = "Jon Sagebrand";
  String email = "jonsagebrand@gmail.com";
*/

/*******************************
  TM1637 4 x 7-segment display
*******************************/
#include <Arduino.h>
#include <TM1637TinyDisplay.h>

const int CLK = 8;
const int DIO = 9;

TM1637TinyDisplay display(CLK, DIO);

uint8_t data[] = { 0x00, 0x00, 0x00, 0x00 };


/*******************************
  Pins
*******************************/
const int pwmPin = 6; // pin that controls the MOSFET
const int thermPin = A0; // pin connected to the thermistor


/*******************************
  Temperatures and thermistor
*******************************/
double setPoint = 200; // this is our goal temperature

const float seriesResistor = 4490; //100000.0; // value of the resistor in serie with the thermistor

const float thermistorNominal = 100000.0; // resistance at 25 degrees C
const float nominalTemperature = 25.0; // temp. for nominal resistance (almost always 25 C)
const float betaCoefficient = 3950.0; // the beta coefficient of the thermistor (usually 3000-4000)

const int numSamples = 10; // how many samples to take and average, more takes longer but is more 'smooth'
int samples[numSamples];

uint8_t i;
float average;
float steinhart;

const float tempOffset = 0;
const float tempFactor = 0.92;


/*******************************
  Plot
*******************************/
const boolean plot = true; // if true, serial output is only values that can be plotted by the Serial Plotter


/*******************************
  PID
*******************************/
#include <PID_v1.h>

double actualTemp = 0.0; // the measured temperature
double PWMvalue;
//double Kp = 2, Ki = 5, Kd = 1; // initial tuning parameters
double aggKp = 3; //7.5;
double aggKi = 0.3; //0.99;
double aggKd = 0; //0.1;

PID myPID(&actualTemp, &PWMvalue, &setPoint, aggKp, aggKi, aggKd, DIRECT);

const boolean useAdaptiveTuning = false;

double consKp = 1; //7.5;
double consKi = 0.3; //0.99;
double consKd = 0; //0.1;

double tempGap;

/*******************************
  Rotary encoder
*******************************/
#include <SimpleRotary.h>

SimpleRotary rotary(3, 4, 5); // CLK, DT , SW

byte rotEncTurn;
byte rotEncPush;

boolean setTemp = false;


void setup() {

  /*******************************
    External AREF
  *******************************/
  //analogReference(EXTERNAL); // connect AREF to 3.3V and use that as VCC, less noisy!

  /*******************************
    Start serial
  *******************************/
  Serial.begin(9600);

  /*******************************
    In- and outputs
  *******************************/
  pinMode(pwmPin, OUTPUT); // this is the output pin for the MOSFET controoling the heater

  /*******************************
    PID setup
  *******************************/
  steinhart = readTemp(); // initialize the variables we're linked to
  actualTemp = (steinhart + tempOffset) * tempFactor;

  myPID.SetMode(AUTOMATIC); //turn the PID on

  if (useAdaptiveTuning) {
    myPID.SetTunings(consKp, consKi, consKd);
  }

  /*******************************
    Rotary encoder setup
  *******************************/
  // Set the trigger to be either a HIGH or LOW pin (Default: HIGH)
  // Note this sets all three pins to use the same state.
  //rotary.setTrigger(HIGH);

  // Set the debounce delay in ms  (Default: 2)
  //rotary.setDebounceDelay(2);

  // Set the error correction delay in ms  (Default: 200)
  //rotary.setErrorDelay(10);

  /*******************************
    LED display
  *******************************/
  display.clear();
  display.setBrightness(BRIGHT_HIGH);

}


void loop() {

  /*******************************
    Read temperature
  *******************************/
  if (!setTemp) {
    steinhart = readTemp();
    actualTemp = (steinhart + tempOffset) * tempFactor;
  }

  /*******************************
    PID control
  *******************************/
  if (!setTemp) {
    if (useAdaptiveTuning) {
    tempGap = abs(setPoint - actualTemp); // distance away from setpoint

    if (tempGap < 10) { //we're close to setpoint, use conservative tuning parameters
      myPID.SetTunings(consKp, consKi, consKd);
    } else { //we're far from setpoint, use aggressive tuning parameters
      myPID.SetTunings(aggKp, aggKi, aggKd);
    }
    }

    myPID.Compute();
    analogWrite(pwmPin, PWMvalue);
  } else {
    analogWrite(pwmPin, 0);
  }

  /*******************************
    Rotary encoder
  *******************************/
  rotEncPush = rotary.push();
  rotEncTurn = rotary.rotate();

  if (rotEncPush != 0 || rotEncTurn != 0) {  // rotary encoder was manipulated
    if ( rotEncPush == 1 ) { // button pushed
      Serial.println();
      Serial.println("---------- Button pushed");
      if (setTemp) { // toggle setTemp
        setTemp = false;
        Serial.println("Leaving setting mode");
        Serial.println();
      } else {
        setTemp = true;
        Serial.println("In setting mode");
        Serial.print("Setpoint: ");
        Serial.println(setPoint);
        Serial.println();
      }
    }

    if (setTemp) {

      if ( rotEncTurn == 1 ) {
        Serial.println("--- Increasing setpoint..");
        setPoint ++;
        Serial.print("Setpoint: ");
        Serial.println(setPoint);
        Serial.println();
      }

      if ( rotEncTurn == 2 ) {
        Serial.println("--- Decreasing setpoint..");
        setPoint --;
        Serial.print("Setpoint: ");
        Serial.println(setPoint);
        Serial.println();
      }
    }
  }

  /*******************************
    UI
  *******************************/
  if (!setTemp) {
    if (plot) {
      Serial.print("\Setpoint:");
      Serial.print(setPoint, 1);
      Serial.print("\tAdafruitTemperature:");
      Serial.print(steinhart);
      Serial.print("\tActual:");
      Serial.print(actualTemp, 1);
      Serial.print("\tPWMvalue:");
      Serial.println(PWMvalue);
    }
  }

  if (!setTemp) {
    display.showNumber(actualTemp, 0); // print temp to 7-segment display
  } else {
    display.showNumber((int)setPoint, true); // print temp to 7-segment display
  }
  delay(300);

}

double readTemp() {

  for (i = 0; i < numSamples; i++) { // take N samples in a row, with a slight delay
    samples[i] = analogRead(thermPin);
    delay(10);
  }

  average = 0;  // average all the samples out
  for (i = 0; i < numSamples; i++) {
    average += samples[i];
  }
  average /= numSamples;

  average = (1023 / average)  - 1;     // (1023/ADC - 1)
  average = seriesResistor / average;  // 100K / (1023/ADC - 1)

  steinhart = average / thermistorNominal;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= betaCoefficient;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (nominalTemperature + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert absolute

  return steinhart;

}
