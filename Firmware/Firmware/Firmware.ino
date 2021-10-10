
#define HEATPIN 5
void setup() {
  // put your setup code here, to run once:
  pinMode(HEATPIN, OUTPUT);
  digitalWrite(HEATPIN, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(5000);
  digitalWrite(HEATPIN, HIGH);
  delay(5000);
  digitalWrite(HEATPIN, LOW);
}
