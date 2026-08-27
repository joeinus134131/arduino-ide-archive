#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// --- Konfigurasi OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
int eyeWidth = 24, eyeHeight = 32, leftEyeX = 30, rightEyeX = 74, eyeY = 16;

// --- Konfigurasi Servo ---
Servo servoGeleng; // Kiri - Kanan
Servo servoAngguk; // Atas - Bawah

const int pinGeleng = 2;
const int pinAngguk = 3;

void setup() {
  // Setup OLED
  Wire.begin(8, 9); 
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) for(;;);
  
  // Setup Servo
  servoGeleng.setPeriodHertz(50); // Frekuensi standar servo analog
  servoAngguk.setPeriodHertz(50);
  servoGeleng.attach(pinGeleng, 500, 2400); // Pulse width min/max SG90
  servoAngguk.attach(pinAngguk, 500, 2400);

  // Posisi awal (Tengah)
  servoGeleng.write(90);
  servoAngguk.write(90);
  
  drawEyes();
}

void loop() {
  delay(2000);
  blink();
  
  // Animasi melihat sekitar
  tengokKiri();
  delay(1000);
  blink();
  
  tengokKanan();
  delay(1000);
  
  posisiTengah();
  delay(2000);
}

// --- Fungsi Animasi Servo ---
void tengokKiri() {
  // Bergerak halus ke sudut 135 derajat
  for(int pos = 90; pos <= 135; pos++) {
    servoGeleng.write(pos);
    delay(15);
  }
}

void tengokKanan() {
  // Bergerak halus dari 135 ke 45 derajat
  for(int pos = 135; pos >= 45; pos--) {
    servoGeleng.write(pos);
    delay(15);
  }
}

void posisiTengah() {
  // Kembali ke 90 derajat
  for(int pos = 45; pos <= 90; pos++) {
    servoGeleng.write(pos);
    delay(15);
  }
}

// --- Fungsi OLED (Sama seperti sebelumnya) ---
void drawEyes() {
  display.clearDisplay();
  display.fillRoundRect(leftEyeX, eyeY, eyeWidth, eyeHeight, 6, SSD1306_WHITE);
  display.fillRoundRect(rightEyeX, eyeY, eyeWidth, eyeHeight, 6, SSD1306_WHITE);
  display.display();
}

void blink() {
  for (int h = eyeHeight; h > 0; h -= 4) {
    display.clearDisplay();
    int yOffset = (eyeHeight - h) / 2;
    display.fillRoundRect(leftEyeX, eyeY + yOffset, eyeWidth, h, 6, SSD1306_WHITE);
    display.fillRoundRect(rightEyeX, eyeY + yOffset, eyeWidth, h, 6, SSD1306_WHITE);
    display.display();
    delay(10);
  }
  delay(30);
  for (int h = 0; h <= eyeHeight; h += 4) {
    display.clearDisplay();
    int yOffset = (eyeHeight - h) / 2;
    display.fillRoundRect(leftEyeX, eyeY + yOffset, eyeWidth, h, 6, SSD1306_WHITE);
    display.fillRoundRect(rightEyeX, eyeY + yOffset, eyeWidth, h, 6, SSD1306_WHITE);
    display.display();
    delay(10);
  }
  drawEyes();
}