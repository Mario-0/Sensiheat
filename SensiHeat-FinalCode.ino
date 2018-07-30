char c;
int rate = 0;
boolean heater = 0;
int outputPin = 3;

//tempOffset: 72.2 for system 2; 0 for system 1
float tempOffset = 0;
// the filter coefficient, 0.1 means a scale of 10 samples, 0.01 would be 100 samples
#define smoothing 0.5  
float temp1 = 0;
float average1 = 0, average2 = 0;
int rawADCTemp1;
int counter = 0;
int i;
float sum = 0;

//array to store temperature measurements
float tempArray[10];
int tempArrayCount = 10;

//array to store temperature averages
float avgArray[10];  // for control samples
int avgArrayCount = 10; // number of elements in control sample array (must be same number as above)

//for calculating and storing differences between temperatures
float tempdiff,oldestsample,newestsample;
float diffArray[2];      //for diff samples
int diffArrayCount = 2;  // number of elements in diff sample array

//heating variables: 250/0.3/400 for system 2
//heating variables: 75/0.4/250 for system 1
int rateMild = 110;
int rateHot = 255;
float mildThreshold = 0.25;
float hotThreshold = 3;
int delayMild = 250;
int delayHot = 1000;


void setup() {
  pinMode(outputPin, OUTPUT);
  Serial.begin(9600);
  ledTest(); //perform a sequence in led13 as confirmation of succesful script upload
  cold(); //start running the code with a cold heating pad
}

void loop() {

  manualControl();

  mainFunction();

  printLogs();

  delay(250);

}

void mainFunction(){
  
  counter = counter + 1;
  
  // reads temp sensor output and converts to temp in celcius
  temp1 = (((float(analogRead(A0)) * 5.0 / 1023 - .5) * 100) - tempOffset); 

  //store individual temperature measurements in array and update array with each new measurement
  for(i=1;i<=10;i++){
    tempArray[i-1]=tempArray[i]; // shift all but last element
  }
  tempArray[10-1]= temp1;

  //calculate average temperature over last 10 measurements
  for(i=1;i<10;i++){
    sum = sum + tempArray[i];
  }
  average1 = sum/9;
  sum = 0;
  
  //store individual averages in array and update array with each new incoming averages
  for(i=1;i<=avgArrayCount;i++){
    avgArray[i-1]=avgArray[i]; // shift all but last element
  }
  avgArray[avgArrayCount-1]= average1;  // assign the most recent value to last element

  // calculate differences between temperature averages
  oldestsample = avgArray[0];
  newestsample = avgArray[avgArrayCount-1];  
  tempdiff = oldestsample - newestsample;

  //make tempDiff positive if negative
  if (tempdiff < 0){
    tempdiff*=-1;
  }

  // update array with temperature differences
  for(i=1;i<=diffArrayCount;i++){
    diffArray[i-1]=diffArray[i]; // shift all but last element
  }
  diffArray[diffArrayCount-1]= tempdiff;  // assign the most recent value to last element


  if (counter > 50) {
    if(diffArray[0] > mildThreshold && diffArray[1] > mildThreshold && diffArray[0] < 5 && diffArray[1] < 5){
      heat(rateMild);
      Serial.print(" mild"); Serial.print(" \t"); 
      delay(delayMild);
    } else {
      cold();
      Serial.print(" off ");
    }
  } 
}

void manualControl(){
  if (Serial.available() > 0) {

    c = Serial.read();

    rate = Serial.parseInt();

    if (c == 'r'){
      heat(rate);
    } else if (c == 'c') {
      cold();
    } else if (c == 'm') {
      mild();
    } else if (c == 'h') {
      hot();
    } else if (c == 'l') {
      ledTest();
    }
  }
}

void printLogs(){
  Serial.print("counter: "); Serial.print(counter); Serial.print(" \t"); 
  Serial.print("avg temp: "); Serial.print(average1); Serial.print(" \t"); 
  Serial.println("");
}

void cold(){
  analogWrite(outputPin, LOW);
}

void mild(){
  analogWrite(outputPin, 120);
  Serial.println("medium");
}

void hot(){
  analogWrite(outputPin, 255);
  Serial.println("hot");
}

void heat(int rate){
  analogWrite(outputPin, rate);
}
