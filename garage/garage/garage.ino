#define RELAY_PIN D1
#define DHT_PIN D4
#define FORCE_PIN  D2
#define SENSE_PIN  D3

volatile bool isr = false;
volatile uint8_t isr_count = 0;

void relay_pulse(void)
{
  digitalWrite(RELAY_PIN, 1);
  delay(1000);
  digitalWrite(RELAY_PIN, 0);
}

void door_opened(void)
{
  isr_count++;
  isr = true;
  Serial.print("ISR "); Serial.println(isr_count);
}


void setup() {
  Serial.begin(9600);
  Serial.println("Starting");
  // put your setup code here, to run once:
  pinMode(SENSE_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(FORCE_PIN, OUTPUT);
  digitalWrite(FORCE_PIN, 0);
  attachInterrupt(digitalPinToInterrupt(SENSE_PIN), door_opened, CHANGE);

  if (digitalRead(SENSE_PIN) ) {
    Serial.println("Door Openned");

  } else {
    Serial.println("Door Closed");
  }


}

void loop() {
  if (isr) {
    delay(2000);
    if (digitalRead(SENSE_PIN) ) {
      Serial.println("Door Openned");
      Serial.println("Closing...");
      relay_pulse();
    } else {
      Serial.println("Door Closed");
    }
    isr = false;
  }
  delay(1000);
  // put your main code here, to run repeatedly:

}
