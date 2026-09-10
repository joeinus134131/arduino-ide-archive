#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// --- Konfigurasi OLED ---
Adafruit_SSD1306 display(128, 64, &Wire, -1);
int eyeW = 24, eyeH = 32, leftX = 30, rightX = 74, eyeY = 16;

// --- Konfigurasi Servo & Pin ---
Servo servoGeleng;
Servo servoAngguk;
const int pinGeleng = 2;
const int pinAngguk = 3;
const int pinTouch = 4;

// Variabel status
bool sedangDisayang = false;
unsigned long waktuTerakhirKedip = 0;

void setup() {
  Serial.begin(115200);
  pinMode(pinTouch, INPUT);

  // Inisialisasi OLED
  Wire.begin(8, 9); 
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) for(;;);
  
  // Inisialisasi Servo
  servoGeleng.setPeriodHertz(50);
  servoAngguk.setPeriodHertz(50);
  servoGeleng.attach(pinGeleng, 500, 2400);
  servoAngguk.attach(pinAngguk, 500, 2400);

  // Posisi siaga (Tengah)
  servoGeleng.write(90);
  servoAngguk.write(90);
  
  drawNormalEyes();
}

// void loop() {
//   int touchState = digitalRead(pinTouch);

//   if (touchState == HIGH) {
//     if (!sedangDisayang) {
//       modeDisayang(); // Trigger mode disayang saat pertama disentuh
//     }
//     // Jika terus disentuh/di-elus, tetap di posisi ini
//     sedangDisayang = true; 
//   } else {
//     if (sedangDisayang) {
//       // Jika sentuhan dilepas, kembali ke normal
//       kembaliNormal();
//       sedangDisayang = false;
//     }
    
//     // Saat tidak disentuh, jalankan rutinitas normal (berkedip sesekali)
//     if (millis() - waktuTerakhirKedip > 3500) { // Kedip setiap 3.5 detik
//       blink();
//       waktuTerakhirKedip = millis();
//     }
//   }
//   delay(20);
// }
// Tambahkan variabel ini di atas setup()
unsigned long waktuTerakhirDisentuh = 0;

void loop() {
  int touchState = digitalRead(pinTouch);

  if (touchState == HIGH) {
    waktuTerakhirDisentuh = millis(); // Reset timer saat disentuh
    
    if (!sedangDisayang) {
      modeDisayang(); 
    }
    sedangDisayang = true; 
  } else {
    if (sedangDisayang) {
      kembaliNormal();
      sedangDisayang = false;
    }
    
    // LOGIKA IDLE (BOSAN)
    if (millis() - waktuTerakhirDisentuh > 10000) { 
      // Jika dicuekin 10 detik, jadi sedih
      modeSedih();
      delay(3000);          // Sedih selama 3 detik
      waktuTerakhirDisentuh = millis(); // Reset timer
      kembaliNormal();
    } 
    // Kedip normal
    else if (millis() - waktuTerakhirKedip > 3500) { 
      blink();
      waktuTerakhirKedip = millis();
    }
  }
  delay(20);
}

// ==========================================
// LOGIKA MODE INTERAKSI
// ==========================================

void modeDisayang() {
  Serial.println("Ahhh... nyaman~");
  
  // 1. Ubah ekspresi mata jadi melengkung senang ( ^ ^ )
  drawHappyEyes();

  // 2. Kepala mendongak ke atas (menikmati elusan)
  // Asumsi 90 tengah, < 90 mendongak ke atas. Sesuaikan dengan arah pasang servo Anda!
  gerakHalus(servoAngguk, 90, 60); 
  
  // 3. Kepala goyang sedikit ke kiri dan kanan (manja)
  gerakHalus(servoGeleng, 90, 100);
  gerakHalus(servoGeleng, 100, 80);
  gerakHalus(servoGeleng, 80, 90);
}

void kembaliNormal() {
  Serial.println("Kembali fokus.");
  // Mata kembali normal
  drawNormalEyes();
  
  // Kepala kembali ke tengah perlahan
  gerakHalus(servoAngguk, servoAngguk.read(), 90);
  gerakHalus(servoGeleng, servoGeleng.read(), 90);
}

// ==========================================
// FUNGSI PENDUKUNG SERVO & OLED
// ==========================================

void gerakHalus(Servo &servo, int posisiAwal, int posisiAkhir) {
  if (posisiAwal < posisiAkhir) {
    for (int p = posisiAwal; p <= posisiAkhir; p++) {
      servo.write(p);
      delay(15);
    }
  } else {
    for (int p = posisiAwal; p >= posisiAkhir; p--) {
      servo.write(p);
      delay(15);
    }
  }
}

void drawNormalEyes() {
  display.clearDisplay();
  display.fillRoundRect(leftX, eyeY, eyeW, eyeH, 6, SSD1306_WHITE);
  display.fillRoundRect(rightX, eyeY, eyeW, eyeH, 6, SSD1306_WHITE);
  display.display();
}

void drawHappyEyes() {
  display.clearDisplay();
  // Menggambar bentuk bulan sabit / melengkung ke atas
  // Mata Kiri
  display.fillCircle(leftX + (eyeW/2), eyeY + (eyeH/2), 12, SSD1306_WHITE);
  display.fillRect(leftX - 5, eyeY + (eyeH/2), eyeW + 10, 20, SSD1306_BLACK); // Potong bagian bawah
  
  // Mata Kanan
  display.fillCircle(rightX + (eyeW/2), eyeY + (eyeH/2), 12, SSD1306_WHITE);
  display.fillRect(rightX - 5, eyeY + (eyeH/2), eyeW + 10, 20, SSD1306_BLACK); // Potong bagian bawah
  display.display();
}

void blink() {
  for (int h = eyeH; h > 0; h -= 4) {
    display.clearDisplay();
    int yOffset = (eyeH - h) / 2;
    display.fillRoundRect(leftX, eyeY + yOffset, eyeW, h, 6, SSD1306_WHITE);
    display.fillRoundRect(rightX, eyeY + yOffset, eyeW, h, 6, SSD1306_WHITE);
    display.display();
    delay(5);
  }
  delay(20);
  for (int h = 0; h <= eyeH; h += 4) {
    display.clearDisplay();
    int yOffset = (eyeH - h) / 2;
    display.fillRoundRect(leftX, eyeY + yOffset, eyeW, h, 6, SSD1306_WHITE);
    display.fillRoundRect(rightX, eyeY + yOffset, eyeW, h, 6, SSD1306_WHITE);
    display.display();
    delay(5);
  }
  drawNormalEyes();
}

void modeMarah() {
  Serial.println("Jangan sentuh aku!");
  
  // Gambar mata marah
  display.clearDisplay();
  display.fillRoundRect(leftX, eyeY, eyeW, eyeH, 6, SSD1306_WHITE);
  display.fillRoundRect(rightX, eyeY, eyeW, eyeH, 6, SSD1306_WHITE);
  
  // Trik masking: Potong sudut dalam atas dengan segitiga hitam
  // Kiri (potong kanan atas)
  display.fillTriangle(leftX, eyeY - 5, leftX + eyeW + 5, eyeY - 5, leftX + eyeW + 5, eyeY + 15, SSD1306_BLACK);
  // Kanan (potong kiri atas)
  display.fillTriangle(rightX - 5, eyeY - 5, rightX + eyeW, eyeY - 5, rightX - 5, eyeY + 15, SSD1306_BLACK);
  display.display();

  // Gerakan leher agresif (geleng cepat)
  servoAngguk.write(90); // Menatap lurus tajam
  for(int i=0; i<3; i++) {
    servoGeleng.write(110);
    delay(100);
    servoGeleng.write(70);
    delay(100);
  }
  servoGeleng.write(90);
}

void modeSedih() {
  Serial.println("Aku kesepian...");
  
  // Gambar mata sedih
  display.clearDisplay();
  display.fillRoundRect(leftX, eyeY, eyeW, eyeH, 6, SSD1306_WHITE);
  display.fillRoundRect(rightX, eyeY, eyeW, eyeH, 6, SSD1306_WHITE);
  
  // Trik masking: Potong sudut luar atas
  // Kiri (potong kiri atas)
  display.fillTriangle(leftX - 5, eyeY - 5, leftX + eyeW, eyeY - 5, leftX - 5, eyeY + 15, SSD1306_BLACK);
  // Kanan (potong kanan atas)
  display.fillTriangle(rightX, eyeY - 5, rightX + eyeW + 5, eyeY - 5, rightX + eyeW + 5, eyeY + 15, SSD1306_BLACK);
  display.display();

  // Gerakan leher sedih (menunduk pelan)
  // Asumsi sudut > 90 adalah menunduk. Sesuaikan jika terbalik di robot Anda.
  gerakHalus(servoAngguk, servoAngguk.read(), 130); 
  delay(1000); // Tahan posisi sedih
}

void modeBingung() {
  Serial.println("Hah? Apa itu?");
  
  display.clearDisplay();
  // Mata kiri normal
  display.fillRoundRect(leftX, eyeY, eyeW, eyeH, 6, SSD1306_WHITE);
  // Mata kanan menyipit (ukurannya setengah)
  display.fillRoundRect(rightX, eyeY + 16, eyeW, eyeH - 16, 6, SSD1306_WHITE);
  display.display();

  // Gerakan leher bingung (menengok ragu-ragu)
  gerakHalus(servoGeleng, 90, 120);
  gerakHalus(servoAngguk, 90, 100);
  delay(1500); // Berpikir sejenak
}