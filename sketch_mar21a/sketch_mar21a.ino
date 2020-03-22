#define relay 13
#define interval 1000

void setup() {
  // put your setup code here, to run once:
  pinMode(relay, OUTPUT);
}

void loop() {
   digitalWrite(relay, HIGH);
   delay(interval);
   digitalWrite(relay, LOW);
   delay(interval);
}
