#include <PID_v1.h>
#include <ht16k33.h>
#include <Wire.h>
#include "SegmentMap.h"

HT16K33 HT;
#define HEATPIN 5
#define THERMOCPL 3
void setup() {
  //HT16K33 begin
  HT.begin(0x00);
  pinMode(HEATPIN, OUTPUT);
  digitalWrite(HEATPIN, LOW);
  pinMode(THERMOCPL, INPUT);

  HT.clearAll();
}

void loop() {
  // put your main code here, to run repeatedly:
  float vtemp = analogRead(THERMOCPL)*3.3/1024;
  int temp = (int)((vtemp-0)/.005);

  int dig1 = temp%10;
  int dig2 = (temp/10)%10;
  int dig3 = (temp/100)%10;

  HT.clearAll();
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
  HT.sendLed();
  delay(1000);
}
