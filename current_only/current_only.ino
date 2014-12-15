// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3

#include "EmonLib.h"                   // Include Emon Library

EnergyMonitor emon1;                   // Create an instance
EnergyMonitor emon2;
EnergyMonitor emon3;


void setup()
{  
  Serial.begin(9600);

  emon1.current(2, 49.3);             // Current: input pin, calibration.
  emon2.current(3, 49.3);
  emon3.current(4, 49.3);
}

void loop()
{

  double Irms1 = emon1.calcIrms(1480);  // Calculate Irms only
  double Irms2 = emon2.calcIrms(1480);
  double Irms3 = emon3.calcIrms(1480);

  Serial.print("1: ");
  //Serial.print(Irms1*230.0);	       // Apparent power
  Serial.print(" ");
  Serial.print(Irms1);		       // Irms
  Serial.print("    ");

  Serial.print("2: ");
  //Serial.print(Irms2*230.0);	       // Apparent power
  Serial.print(" ");
  Serial.print(Irms2);		       // Irms
  Serial.print("    ");

  Serial.print("1: ");
  //Serial.print(Irms3*230.0);	       // Apparent power
  Serial.print(" ");
  Serial.print(Irms3);		       // Irms
  Serial.println("    ");
}


