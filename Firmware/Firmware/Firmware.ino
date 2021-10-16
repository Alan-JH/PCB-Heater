#include <PID_v1.h>
#include <ht16k33.h>
#include <Wire.h>
#include "SegmentMap.h"
#include "ReflowProfile.h"

HT16K33 HT;
#define HEATPIN 5 //PWM out pin for heatplate driver MOSFET
#define THERMOCPL 3 //Analog in pin for thermocouple/AD8497 input

#define DISPINTERVAL 20 //How many loop iterations before display is updated
int dispct = 0; //Counter value to update display

double setPoint, Input, Output; //PID values
int reflowStage = 0; //Current reflow stage
unsigned long lastStageTime = 0; //last stage change time in ms
unsigned long lastRecordTime = 0; //generic last time variable, in ms
bool pastPeak = false; //whether the temperature has passed peak temp

double kp, ki, kd; //PID parameters to tune

PID HeatPID(&Input, &Output, &setPoint, kp, ki, kd, DIRECT);

void setup() {
  //HT16K33 begin
  HT.begin(0x00);
  HT.clearAll();

  //Initialize pins, set heater to low immediately
  pinMode(HEATPIN, OUTPUT);
  digitalWrite(HEATPIN, LOW);
  pinMode(THERMOCPL, INPUT);

  //Start PID
  HeatPID.SetMode(AUTOMATIC);
  setPoint = 24;

  //Start profile (remove once start button is added)
  startProfile();
}

void setDisp1(int temp){ //Updates display 1: actual temperature
  int dig1 = temp%10;
  int dig2 = (temp/10)%10;
  int dig3 = (temp/100)%10;

  uint8_t dig1bin = 0;
  uint8_t dig2bin = 0;
  uint8_t dig3bin = 0;
  for (int i = 0; i < 7; i++){
    if (charMap[dig1][i] == 1)
      dig1bin |= segDigMap[2][i][1];
    if (charMap[dig2][i] == 1)
      dig2bin |= segDigMap[1][i][1];
    if (charMap[dig3][i] == 1)
      dig3bin |= segDigMap[0][i][1];
  }
  HT.set7SegRaw(2, dig1bin);
  HT.set7SegRaw(1, dig2bin);
  HT.set7SegRaw(0, dig3bin);
}

void setDisp2(int target){ //Updates display 2: target temperature
  int dig1 = target%10;
  int dig2 = (target/10)%10;
  int dig3 = (target/100)%10;

  uint8_t dig1bin = 0;
  uint8_t dig2bin = 0;
  uint8_t dig3bin = 0;
  for (int i = 0; i < 7; i++){
    if (charMap[dig1][i] == 1)
      dig1bin |= segDigMap[8][i][1];
    if (charMap[dig2][i] == 1)
      dig2bin |= segDigMap[7][i][1];
    if (charMap[dig3][i] == 1)
      dig3bin |= segDigMap[6][i][1];
  }
  HT.set7SegRaw(3, dig1bin);
  HT.set7SegRaw(4, dig2bin);
  HT.set7SegRaw(5, dig3bin);
}

void startProfile(){ //Starts reflow profile
  lastStageTime = millis();
  kp = 2.5;
  ki = 0.5;
  kd = 5;
  reflowStage = 0;
  setPoint = stageTemps[0];
}

void updateProfile(){ //Updates reflow profile, changes stages, updates setpoint if needed
  switch (reflowStage){
    case 0: //First stage ramp rate doesn't really matter, so just set the setpoint in startProfile() and move to next stage when the time comes
      if ((millis()-lastStageTime)/1000 > stageTimes[0]){
        reflowStage ++;
        lastStageTime = millis();
        lastRecordTime = millis();
      }
      break;
    case 1: //Second stage needs to maintain a relatively slow ramp, so step the setpoint up at constant rate over time until next stage
      if (millis() - lastRecordTime > 500){
        setPoint += ((0.5)*(stageTemps[1]-stageTemps[0])/(stageTimes[1]));
        lastRecordTime = millis();
      }
      if ((millis()-lastStageTime)/1000 > stageTimes[1]){
        reflowStage ++;
        lastStageTime = millis();
      }
      break;
    case 2: //Third stage ramps to peak temperature as quickly as possible, and maintains peak for set amount of time
      setPoint = stageTemps[2];
      kp = 10;
      ki = 4;
      if (Input > PEAKTHRESHOLD && !pastPeak){
        lastRecordTime = millis();
        pastPeak = true;
      }
      if (pastPeak && (millis() - lastRecordTime)/1000 > stageTimes[2]){
        reflowStage ++;
        lastStageTime = millis();
      }
      break;
    case 3: //Cooldown, may incorporate fan usage later
      setPoint = 0;
      break;
  }
}

void loop() {
  float vtemp = analogRead(THERMOCPL)*3.3/1024; //raw voltage
  int temp = (int)((vtemp-0)/.005); //Convert to output with 0 VREF and 5mV/C step

  if (dispct == 0){ //Update displays if interval = 0
    HT.clearAll();
    setDisp1(temp);
    setDisp2((int)setPoint);
    dispct = DISPINTERVAL;
    HT.sendLed();
  }
  dispct --;

  Input = temp; //Set input
  HeatPID.Compute(); //Compute PID, write output
  analogWrite(HEATPIN, Output);

  updateProfile();
  delay(50);
}
