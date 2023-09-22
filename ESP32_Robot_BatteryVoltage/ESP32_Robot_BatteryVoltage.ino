
// R1 = 12k (in)
// R2 = 2k (pulldown)
void setup() {
  Serial.begin(115200);

  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
}

float multiplier = 3.352941176;

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  int sense1 = analogReadMilliVolts(A1);
  int sense2 = analogReadMilliVolts(A2);

  Serial.println(String((sense1 - sense2) * multiplier) + "\t" + String(sense2 * multiplier));
  delay(40);
}
