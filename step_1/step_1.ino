#define LED_PIN 13
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()>0) {
    char command = Serial.read();

    Serial.println("ini command :" + command);
    if (command == '1') {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("Arduino: Lampu ON");
    } else if (command == '0') {
      digitalWrite(LED_PIN, LOW);
      Serial.println("Arduino: Lampu OFF");
    }
  }
}
