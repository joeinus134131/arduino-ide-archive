// Motor A connections
int enA = 14;
int in1 = 27;
int in2 = 26;

// Motor B connections
int enB = 32;
int in3 = 25;
int in4 = 33;

void setup() {
  // Set all the motor control pins to outputs
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  
  // Turn off motors initially
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

void loop() {
  // Set motor speed (0 is off, 255 is max speed)
  analogWrite(enA, 200); 
  analogWrite(enB, 200);

  // --- MOVE FORWARD ---
  // digitalWrite(in1, HIGH);
  // digitalWrite(in2, LOW);
  // digitalWrite(in3, HIGH);
  // digitalWrite(in4, LOW);
  // delay(2000); // Run for 2 seconds

  // --- SET ARAH MAJU ---
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);

  // --- SOFT START: Naikkan kecepatan secara bertahap ---
  // Dari 0 sampai 255, nambah 5 poin setiap 20 milidetik
  for(int speed = 0; speed <= 255; speed += 5) {
    analogWrite(enA, speed);
    analogWrite(enB, speed);
    delay(20); 
  }

  // Setelah kecepatan penuh, biarkan jalan 2 detik
  delay(2000); 

  // --- STOP ---
  analogWrite(enA, 0);
  analogWrite(enB, 0);
  delay(1000);

  // --- STOP ---
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  delay(1000); // Stop for 1 second

  // --- MOVE BACKWARD ---
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  delay(2000); // Run for 2 seconds
  
  // --- STOP ---
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  delay(1000); // Stop for 1 second
}