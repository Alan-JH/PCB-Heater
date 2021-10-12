#include <PID_v1.h>
#include <ht16k33.h>
#include <Wire.h>
#include "SegmentMap.h"
#include "ReflowProfile.h"

HT16K33 HT;
#define HEATPIN 5
#define THERMOCPL 3

#define DISPINTERVAL 20
int dispct = 0;

double setPoint, Input, Output;
int reflowStage = 0;
unsigned long lastStageTime = 0;
unsigned long lastRecordTime = 0;

double kp = 2, ki = 5, kd = 1;

PID HeatPID(&Input, &Output, &setPoint, kp, ki, kd, DIRECT);

void setup() {
  //HT16K33 begin
  HT.begin(0x00);
  pinMode(HEATPIN, OUTPUT);
  digitalWrite(HEATPIN, LOW);
  pinMode(THERMOCPL, INPUT);
  
  HT.clearAll();

  //Start PID
  HeatPID.SetMode(AUTOMATIC);
  setPoint = 24;

  startProfile();
}

void setDisp1(int temp){
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

void setDisp2(int target){
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

void startProfile(){
  lastStageTime = millis();
  setPoint = stageTemps[0];
}

void updateProfile(){
  switch (reflowStage){
    case 0:
      if ((millis()-lastStageTime)/1000 > stageTimes[0]){
        reflowStage ++;
        lastStageTime = millis();
        lastRecordTime = millis();
      }
      break;
    case 1:
      setPoint += ((millis() - lastRecordTime)*(float)(stageTemps[1]-stageTemps[0])/(1000*(stageTimes[1])));
      lastRecordTime = millis();
      if ((millis()-lastStageTime)/1000 > stageTimes[1]){
        reflowStage ++;
        lastStageTime = millis();
      }
      break;
    case 2:
      setPoint = stageTemps[2];
      if (Input > PEAKTHRESHOLD){
        lastRecordTime = millis();
      }
      if (millis() > lastRecordTime + 1000*stageTimes[2]){
        reflowStage ++;
        lastStageTime = millis();
      }
      break;
    case 3:
      setPoint = 0;
      break;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  float vtemp = analogRead(THERMOCPL)*3.3/1024;
  int temp = (int)((vtemp-0)/.005);

  if (dispct == 0){
    HT.clearAll();
    setDisp1(temp);
    setDisp2((int)setPoint);
    dispct = DISPINTERVAL;
    HT.sendLed();
  }
  dispct --;

  Input = temp;
  HeatPID.Compute();
  analogWrite(HEATPIN, Output);

  updateProfile();
  delay(50);
}
