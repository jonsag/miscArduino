// https://github.com/br3ttb/Arduino-PID-AutoTune-Library

#include <PID_v1.h>
#include <PID_AutoTune_v0.h>

byte ATuneModeRemember = 2;
double input = 80, output = 50, setpoint = 180;
double kp = 2, ki = 0.5, kd = 2;

double kpmodel = 1.5, taup = 100, theta[50];
double outputStart = 5;
double aTuneStep = 50, aTuneNoise = 1, aTuneStartValue = 100;
unsigned int aTuneLookBack = 20;

boolean tuning = false;
unsigned long  modelTime, serialTime;

PID myPID(&input, &output, &setpoint, kp, ki, kd, DIRECT);
PID_ATune aTune(&input, &output);

/*******************************
  Pins
*******************************/
const int pwmPin = 6; // pin that controls the MOSFET
const int thermPin = A0; // pin connected to the thermistor

/*******************************
  Simulation
*******************************/
boolean useSimulation = true; // set to false to connect to the real world

/*******************************
  Temperatures and thermistor
*******************************/
//double setPoint = 200; // this is our goal temperature

const float seriesResistor = 100000.0; // value of the resistor in serie with the thermistor

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

void setup()
{
  if (useSimulation)
  {
    for (byte i = 0; i < 50; i++)
    {
      theta[i] = outputStart;
    }
    modelTime = 0;
  }
  
  myPID.SetMode(AUTOMATIC); // setup the pid

  if (tuning) {
    tuning = false;
    changeAutoTune();
    tuning = true;
  }

  serialTime = 0;
  Serial.begin(9600);

}

void loop()
{

  unsigned long now = millis();

  if (!useSimulation) { //pull the input in from the real world
    steinhart = readTemp(); // initialize the variables we're linked to
    input = (steinhart + tempOffset) * tempFactor;
  }

  if (tuning) {
    byte val = (aTune.Runtime());
    if (val != 0) {
      tuning = false;
    }
    if (!tuning) { //we're done, set the tuning parameters
      kp = aTune.GetKp();
      ki = aTune.GetKi();
      kd = aTune.GetKd();
      myPID.SetTunings(kp, ki, kd);
      AutoTuneHelper(false);
    }
  } else {
    myPID.Compute();
  }

  if (useSimulation) {
    theta[30] = output;
    if (now >= modelTime) {
      modelTime += 100;
      DoModel();
    }
  } else {
    analogWrite(0, output);
  }

  //send-receive with processing if it's time
  if (millis() > serialTime) {
    SerialReceive();
    SerialSend();
    serialTime += 500;
  }
}

void changeAutoTune() {
  if (!tuning) {
    //Set the output to the desired starting frequency.
    output = aTuneStartValue;
    aTune.SetNoiseBand(aTuneNoise);
    aTune.SetOutputStep(aTuneStep);
    aTune.SetLookbackSec((int)aTuneLookBack);
    AutoTuneHelper(true);
    tuning = true;
  } else { //cancel autotune
    aTune.Cancel();
    tuning = false;
    AutoTuneHelper(false);
  }
}

void AutoTuneHelper(boolean start) {
  if (start) {
    ATuneModeRemember = myPID.GetMode();
  } else {
    myPID.SetMode(ATuneModeRemember);
  }
}


void SerialSend() {
  Serial.print("setpoint: "); Serial.print(setpoint); Serial.print(" ");
  Serial.print("input: "); Serial.print(input); Serial.print(" ");
  Serial.print("output: "); Serial.print(output); Serial.print(" ");
  if (tuning) {
    Serial.println("tuning mode");
  } else {
    Serial.print("kp: "); Serial.print(myPID.GetKp()); Serial.print(" ");
    Serial.print("ki: "); Serial.print(myPID.GetKi()); Serial.print(" ");
    Serial.print("kd: "); Serial.print(myPID.GetKd()); Serial.println();
  }
}

void SerialReceive() {
  if (Serial.available()) {
    char b = Serial.read();
    Serial.flush();
    if ((b == '1' && !tuning) || (b != '1' && tuning)) {
      changeAutoTune();
    }
  }
}

void DoModel() {
  //cycle the dead time
  for (byte i = 0; i < 49; i++) {
    theta[i] = theta[i + 1];
  }
  //compute the input
  input = (kpmodel / taup) * (theta[0] - outputStart) + input * (1 - 1 / taup) + ((float)random(-10, 10)) / 100;

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
