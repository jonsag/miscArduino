//const int phaseR1 = A2; // declare the analog inputs
//const int phaseS2 = A3;
//const int phaseT3 = A4;

//const int R1 = A2; // declare the analog inputs
//const int S2 = A3;
//const int T3 = A4;

const int phase[] = {A2,A3,A4};

double displayedCurrent[] = { // the displayed value on the web page
  1, 2, 3};
double ackDisplayedCurrent[] = { // all reads ackumulated
  1, 2, 3};
double measure = 0;

int currentDisplayedCounter = 0;
byte addCurrents = 0;
byte phaseCount = 0;

void setup() {
  Serial.begin(9600); // start serial communication
  Serial.println("experimental_current_reading");
  Serial.println("Started serial communication");
  
  Serial.print("1: ");
  Serial.println(phase[0]);
  Serial.print("2: ");
  Serial.println(phase[1]);
  Serial.print("3: ");
  Serial.println(phase[2]);
  
}


void loop() {
  if (currentDisplayedCounter < 100) {
    
    //Serial.print("Measurement #");
    //Serial.print(currentDisplayedCounter);
    //Serial.print(" ");
    //delay(10);
    
    for(phaseCount = 0; phaseCount <= 2; phaseCount++) {
      //measure = analogRead(phase[phaseCount]);
      measure = abs((analogRead(phase[phaseCount]) - 511) * 0.35);
      ackDisplayedCurrent[phaseCount] = ackDisplayedCurrent[phaseCount] + measure;
      //Serial.print(phase[phaseCount]);
      //Serial.print(": ");
      //Serial.print(measure);
      //Serial.print(" A    ");
    }
    //Serial.println();
    
    //ackDisplayedCurrent[1] = ackDisplayedCurrent[1] + abs((analogRead(phaseR1) - 511) * 0.35); // read currents
    //ackDisplayedCurrent[2] = ackDisplayedCurrent[2] + abs((analogRead(phaseS2) - 511) * 0.35);
    //ackDisplayedCurrent[3] = ackDisplayedCurrent[3] + abs((analogRead(phaseT3) - 511) * 0.35);
    
    currentDisplayedCounter++;
  }
  else {
    Serial.print("Reading #");
    
    Serial.print("  ");
    for(addCurrents=0; addCurrents <= 2; addCurrents++) {
      displayedCurrent[addCurrents] = ackDisplayedCurrent[addCurrents] / 100; // set the displayed current value
      Serial.print(addCurrents);
      Serial.print(": ");
      Serial.print(displayedCurrent[addCurrents]);
      Serial.print(" A     ");
      ackDisplayedCurrent[addCurrents] = 0;
    }
    Serial.println();
    currentDisplayedCounter = 0;
  }
}

