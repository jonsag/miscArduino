/*
  String programName = "ardFilaSplice";
  String date = "20201226";
  String author = "Jon Sagebrand";
  String email = "jonsagebrand@gmail.com";
*/

/*******************************
  TM1637 4 x 7-segment display
*******************************/
/*
  #include <TM1637Display.h> // https://github.com/avishorp/TM1637
  #define CLK 8
  #define DIO 9
  TM1637Display display(CLK, DIO);
*/

#include <TM1637TinyDisplay.h> // 
#define CLK 8
#define DIO 9
TM1637TinyDisplay display(CLK, DIO);

/*******************************
  Pins
*******************************/
const int pwmPin = 6; // pin that controls the MOSFET
const int thermPin = A0; // pin connected to the thermistor


/*******************************
  Temperatures and thermistor
*******************************/
float setPoint = 100;
const float seriesResistor = 4700.0;

int Vo; // this is the voltage at the thermistor analog input
float R2; // calculated resistance of the thermistor

const float thermistorNominal = 100000.0; // resistance at 25 degrees C
const float nominalTemperature = 25.0; // temp. for nominal resistance (almost always 25 C)
const float betaCoefficient = 3950.0; // the beta coefficient of the thermistor (usually 3000-4000)
const int numSamples = 5; // how many samples to take and average, more takes longer but is more 'smooth'
int samples[numSamples];

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
const int kp = 9.1;
const int ki = 0.3;
const int kd = 1.8;

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

}


void loop() {

  /*******************************
    Set MOSFET
  *******************************/
  //digitalWrite(pwmPin, HIGH); // code below switches the heater on and off every second
  //delay(1000);
  //digitalWrite(pwmPin, LOW);
  //delay(1000);


  /*******************************
    Read temperature
  *******************************/
  Vo = analogRead(thermPin); // this reads the analog value of analog input A0
  R2 = seriesResistor * (1023.0 / (float)Vo - 1.0); // seriesResistor is the 4.7 kOhm resistor
  actualTemp = (1.0 / (1.009249522e-03 + 2.378405444e-04 * log(R2) + 2.019202697e-07 * pow(log(R2), 3)) - 273.15);


  /*******************************
    Adafruit read temperature
  *******************************/
  uint8_t i;
  float average;

  for (i = 0; i < numSamples; i++) { // take N samples in a row, with a slight delay
    samples[i] = analogRead(thermPin);
    delay(10);
  }

  average = 0;  // average all the samples out
  for (i = 0; i < numSamples; i++) {
    average += samples[i];
  }
  average /= numSamples;

  if (plot) {
    Serial.print("AverageAnalogReading:");
    Serial.print(average);
  }

  average = (1023 / average)  - 1;     // (1023/ADC - 1)
  average = seriesResistor / average;  // 100K / (1023/ADC - 1)

  float steinhart;
  steinhart = average / thermistorNominal;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= betaCoefficient;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (nominalTemperature + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert absolute

  if (plot) {
    Serial.print("\tThermistorResistance;");
    Serial.print(average);
    Serial.print("\tAdafruitTemperature:");
    Serial.print(steinhart);
  }


  /*******************************
    PID control
  *******************************/
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

  analogWrite(pwmPin, 255 - PIDvalue); // write the PWM signal to the mosfet
  previousPIDerror = PIDerror; // store the previous error for next loop.


  /*******************************
    Visual output
  *******************************/
  if (plot) {
    Serial.print("\tSetpoint:");
    Serial.print(setPoint, 1);
    Serial.print("\tActual:");
    Serial.print(actualTemp, 1);
    Serial.print("\tPWMvalue:");
    Serial.println(255 - PIDvalue);
  } else {
    Serial.print("Setpoint: ");
    Serial.print(setPoint, 1);
    Serial.print(", \tActual: ");
    Serial.print(actualTemp, 1);
    Serial.print(", \tPWM value: ");
    Serial.println(255 - PIDvalue);
  }
  display.showNumber(actualTemp); // print actual temp to display

  delay(300);

}
