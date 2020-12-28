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
#include <TM1637TinyDisplay.h> // 
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
float setPoint = 200;
const float seriesResistor = 100000.0; //4700.0;

const float thermistorNominal = 100000.0; // resistance at 25 degrees C
const float nominalTemperature = 25.0; // temp. for nominal resistance (almost always 25 C)
const float betaCoefficient = 3950.0; // the beta coefficient of the thermistor (usually 3000-4000)
const int numSamples = 10; // how many samples to take and average, more takes longer but is more 'smooth'
int samples[numSamples];

const float tempOffset = 0;
const float tempFactor = 0.90;

float steinhart;
uint8_t i;
float average;


/*******************************
  Plot
*******************************/
const boolean plot = true; // if true, serial output is only values that can be plotted by the Serial Plotter


/*******************************
  PID
*******************************/
float actualTemp = 0.0; // the measured temperature
float PIDerror = 0; // the deviation from setpoint
float previousPIDerror = 0;
float elapsedTime;
float Time;
float prevTime;
int PIDvalue = 0;

//PID constants
//
const int kp = 7.5; //23.12; //9.1;
const int ki = 0.99; //1.72; //0.3;
const int kd = 0.1; //77.65; //1.8;

int PIDp = 0;
int PIDi = 0;
int PIDd = 0;


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
  //TCCR2B = TCCR2B & B11111000 | 0x03;    // set pin 3 and 11 PWM frequency to 980.39 Hz, this is the default for D5 and D6
  Time = millis();

  display.clear();
  display.setBrightness(BRIGHT_HIGH);

}


void loop() {

  /*******************************
    Adafruit read temperature
  *******************************/

  steinhart = readTemp();

  actualTemp = (steinhart + tempOffset) * tempFactor;


  /*******************************
    PID control
  *******************************/
  PIDvalue = doPID();

  analogWrite(pwmPin, PIDvalue); // write the PWM signal to the mosfet
  previousPIDerror = PIDerror; // store the previous error for next loop.


  /*******************************
    Visual output
  *******************************/
  if (plot) {
    Serial.print("ThermistorResistance->");
    Serial.print(average);

    /*
    Serial.print("\tPIDp:");
    Serial.print(PIDp);
    Serial.print("\tPIDi:");
    Serial.print(PIDi);
    Serial.print("\tPIDd:");
    Serial.print(PIDd);
    */
    
    Serial.print("\tAdafruitTemperature:");
    Serial.print(steinhart);
    Serial.print("\tSetpoint:");
    Serial.print(setPoint, 1);
    Serial.print("\tActual:");
    Serial.print(actualTemp, 1);
    Serial.print("\tPWMvalue:");
    Serial.println(PIDvalue);
  } else {
    Serial.print("Setpoint: ");
    Serial.print(setPoint, 1);
    Serial.print(", \tActual: ");
    Serial.print(actualTemp, 1);
    Serial.print(", \tPWM value: ");
    Serial.println(255 - PIDvalue);
  }

  display.showNumber(actualTemp, 0);

  delay(300);

}

float readTemp() {

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

int doPID() {
  
  PIDerror = setPoint - actualTemp; // calculate the error between the setpoint and the real value
  PIDp = kp * PIDerror; // calculate the P value
  if (-3 < PIDerror < 3) { // calculate the I value in a range on +-3
    PIDi = PIDi + (ki * PIDerror);
  }

  prevTime = Time; // for derivative we need real time to calculate speed change rate, the previous time is stored before the actual time read
  Time = millis(); // actual time read
  elapsedTime = (Time - prevTime) / 1000;
  PIDd = kd * ((PIDerror - previousPIDerror) / elapsedTime); // calculate the D value
  PIDvalue = PIDp + PIDi + PIDd; // total PID value is the sum of P + I + D

  if (PIDvalue < 0) { // define PWM range between 0 and 255
    PIDvalue = 0;
  }
  if (PIDvalue > 255) {
    PIDvalue = 255;
  }

  return PIDvalue;
  
}
