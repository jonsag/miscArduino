/*
  String programName = "ardFilaSplice";
  String date = "20201226";
  String author = "Jon Sagebrand";
  String email = "jonsagebrand@gmail.com";
*/

const int pwmPin = 6;
float setPoint = 100;


/*******************************
  Plot
*******************************/
const boolean plot = false; // if true, serial output is only values that can be plotted by the Serial Plotter


/*******************************
  Thermistor
*******************************/
int Vo;
float R2, T;


/*******************************
  PID
*******************************/
//Variables
float actualTemp = 0.0;
float PID_error = 0;
float previous_error = 0;
float elapsedTime, Time, timePrev;
int PID_value = 0;

//PID constants
const int kp = 9.1;
const int ki = 0.3;
const int kd = 1.8;

int PID_p = 0;
int PID_i = 0;
int PID_d = 0;


void setup() {
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
  Vo = analogRead(0); // this reads the analog value of analog input A0
  R2 = 4700.0 * (1023.0 / (float)Vo - 1.0); // 4700.0 is the 4.7 kOhm resistor
  T = (1.0 / (1.009249522e-03 + 2.378405444e-04 * log(R2) + 2.019202697e-07 * pow(log(R2), 3)) - 273.15);


  /*******************************
    PID control
  *******************************/
  PID_error = setPoint - T; // calculate the error between the setpoint and the real value
  PID_p = kp * PID_error; // calculate the P value
  if (-3 < PID_error < 3) { // calculate the I value in a range on +-3
    PID_i = PID_i + (ki * PID_error);
  }

  timePrev = Time; // for derivative we need real time to calculate speed change rate, the previous time is stored before the actual time read
  Time = millis(); // actual time read
  elapsedTime = (Time - timePrev) / 1000;
  PID_d = kd * ((PID_error - previous_error) / elapsedTime); // calculate the D value
  PID_value = PID_p + PID_i + PID_d; // total PID value is the sum of P + I + D

  if (PID_value < 0) { // define PWM range between 0 and 255
    PID_value = 0;
  }
  if (PID_value > 255) {
    PID_value = 255;
  }

  analogWrite(pwmPin, 255 - PID_value); // write the PWM signal to the mosfet
  previous_error = PID_error; // store the previous error for next loop.

  delay(300);
  Serial.print("Setpoint:");
  Serial.print(setPoint, 1);
  Serial.print(", Actual:");
  Serial.print(actualTemp, 1);
  Serial.print(", PWM value: ");
  Serial.println(255 - PID_value);

}
