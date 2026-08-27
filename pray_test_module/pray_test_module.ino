#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

// Inisialisasi Objek
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

// Definisi Pin Relay
const int RELAY_PIN = 2;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Inisialisasi Relay OFF (Active LOW)

  // Inisialisasi LCD
  lcd.init();
  lcd.backlight();

  // Inisialisasi RTC
  if (!rtc.begin()) {
    lcd.setCursor(0, 0);
    lcd.print("RTC Tidak ADA!");
    while (1); // Berhenti jika RTC tidak terdeteksi
  }

  // --- CATATAN ATUR WAKTU ---
  // Jika RTC baru/baterai habis, hilangkan tanda // pada baris di bawah ini
  // untuk mensinkronkan waktu RTC dengan waktu kompilasi laptop/PC.
  // Setelah sekali di-upload, beri tanda // kembali lalu upload ulang.
  
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  lcd.setCursor(0, 0);
  lcd.print("System Ready...");
  delay(1500);
  lcd.clear();
}

void loop() {
  DateTime now = rtc.now();

  // 1. Tampilkan Jam di Baris Pertama (HH:MM:SS)
  lcd.setCursor(0, 0);
  lcd.print("Jam: ");
  if (now.hour() < 10) lcd.print('0');
  lcd.print(now.hour());
  lcd.print(':');
  if (now.minute() < 10) lcd.print('0');
  lcd.print(now.minute());
  lcd.print(':');
  if (now.second() < 10) lcd.print('0');
  lcd.print(now.second());

  // 2. Logika Kontrol Relay Berdasarkan Waktu
  // Contoh: Relay menyala ON di jam 18:00 (6 sore) hingga jam 06:00 (6 pagi)
  bool relayShouldBeOn = (now.hour() >= 18 || now.hour() < 6);

  if (relayShouldBeOn) {
    digitalWrite(RELAY_PIN, LOW);  // Relay ON
    lcd.setCursor(0, 1);
    lcd.print("Relay: ON  (AUTO)");
  } else {
    digitalWrite(RELAY_PIN, HIGH); // Relay OFF
    lcd.setCursor(0, 1);
    lcd.print("Relay: OFF (AUTO)");
  }

  delay(500); // Refresh tampilan setiap 0.5 detik
}