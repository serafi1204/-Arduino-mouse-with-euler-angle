/*
Sub Device for mouse switch 
*/

#define SW0 2 // mouse left switch
#define SW1 3 // mouse right switch=
// data for comm
// data is 3bit value
// data = {sw0, sw1}
int sw = 0;

void setup() {
  // Serial setting
  Serial.begin(9600);

  // PIN setting
  pinMode(SW0, INPUT_PULLUP);
  pinMode(SW1, INPUT_PULLUP);
}

void loop() {
  sw = 0;
  for (int i=SW0; i<=SW1; i++) {
    sw += digitalRead(i) == true ? 0 : 1<<(i-SW0);
  }
  Serial.println(sw);
  delay(100);
}
