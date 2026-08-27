/*
 *  PROJECT PENGINGAT SEMBAHYANG OTOMATIS (v2 - IDNmakerspace Algorithm Factory)
 *  Dikembangkan dari kode lama, diperbaiki & ditambah:
 *    - RTC DS3231 (bukan DS1307)
 *    - LCD 16x2 I2C (tampilan jam & status)
 *    - DFPlayer Mini MP3 (per alarm bisa beda track suara)
 *    - Relay ON selama durasi tertentu, non-blocking (pakai millis, bukan delay)
 *    - Kalibrasi RTC pakai 3 tombol fisik (MODE / UP / DOWN) -> tidak perlu re-upload kode
 *
 *  ==================== WIRING (silakan sesuaikan) ====================
 *  RTC DS3231   -> SDA = A4, SCL = A5 (I2C, Arduino Uno/Nano)
 *  LCD16x2 I2C  -> SDA = A4, SCL = A5 (sharing bus sama RTC, alamat default 0x27)
 *  DFPlayer Mini-> RX modul ke pin 11 (TX Arduino), TX modul ke pin 10 (RX Arduino)
 *  Relay        -> pin 7
 *  LED indikator-> pin 13
 *  Tombol MODE  -> pin 2  (ke GND, pakai INPUT_PULLUP)
 *  Tombol UP    -> pin 3  (ke GND, pakai INPUT_PULLUP)
 *  Tombol DOWN  -> pin 4  (ke GND, pakai INPUT_PULLUP)
 *  =======================================================================
 *
 *  ==================== LIBRARY YANG DIPERLUKAN (install via Library Manager) ====
 *   - "RTClib" by Adafruit
 *   - "LiquidCrystal I2C" by Frank de Brabander (atau johnrickman)
 *   - "DFRobotDFPlayerMini" by DFRobot
 *  =======================================================================
 *
 *  CARA KALIBRASI JAM (tanpa re-upload kode):
 *   1. Tekan tombol MODE -> masuk mode kalibrasi, field pertama (TANGGAL) berkedip.
 *   2. Tekan UP / DOWN untuk ubah nilai field yang sedang aktif.
 *   3. Tekan MODE lagi untuk pindah ke field berikutnya
 *      (Tanggal -> Bulan -> Tahun -> Jam -> Menit -> Detik).
 *   4. Setelah field Detik, tekan MODE sekali lagi -> waktu otomatis DISIMPAN ke RTC
 *      dan layar kembali ke tampilan normal.
 */

#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// ---------------- KONFIGURASI PIN ----------------
const uint8_t PIN_RELAY   = 7;
const uint8_t PIN_LED     = 13;
const uint8_t PIN_BTN_MODE = 2;
const uint8_t PIN_BTN_UP   = 3;
const uint8_t PIN_BTN_DOWN = 4;
const uint8_t PIN_DFP_RX   = 10; // ke TX modul DFPlayer
const uint8_t PIN_DFP_TX   = 11; // ke RX modul DFPlayer

// ---------------- KONFIGURASI ALARM (silakan ubah sesuai kebutuhan) ----------------
struct Alarm {
  uint8_t hour;
  uint8_t minute;
  uint8_t track;        // nomor track mp3 di SD card DFPlayer, misal 1.mp3, 2.mp3
  const char* label;
  bool triggeredToday;  // flag internal, jangan diubah manual
};

Alarm alarms[3] = {
  { 6,  0, 1, "Pagi",  false},  // sembahyang pagi  06:00, track 1
  {12,  0, 1, "Siang", false},  // sembahyang siang 12:00, track 2
  {18,  0, 1, "Sore",  false},  // sembahyang sore  18:00, track 3
};

const unsigned long RELAY_ON_DURATION_MS = 7UL * 60UL * 1000UL; // relay nyala 7 menit
const uint8_t MP3_VOLUME = 25; // 0-30

// ---------------- OBJEK GLOBAL ----------------
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2); // kalau layar blank, coba ganti alamat ke 0x3F
SoftwareSerial dfSerial(PIN_DFP_RX, PIN_DFP_TX);
DFRobotDFPlayerMini mp3;

const char* namaHari[7] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};

// state relay non-blocking
bool relayActive = false;
unsigned long relayOnAt = 0;

// state tampilan (biar LCD tidak di-refresh tiap loop, cukup tiap detik)
unsigned long lastLcdUpdate = 0;

// ---------------- STATE KALIBRASI ----------------
// fieldIndex: -1 = mode normal, 0=Tanggal,1=Bulan,2=Tahun,3=Jam,4=Menit,5=Detik
int8_t calibField = -1;
int calibDay, calibMonth, calibYear, calibHour, calibMinute, calibSecond;

// debounce tombol
unsigned long lastBtnPress = 0;
const unsigned long DEBOUNCE_MS = 200;

void setup() {
  Serial.begin(9600);

  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN_MODE, INPUT_PULLUP);
  pinMode(PIN_BTN_UP, INPUT_PULLUP);
  pinMode(PIN_BTN_DOWN, INPUT_PULLUP);

  digitalWrite(PIN_RELAY, LOW); // pastikan relay OFF saat start
  digitalWrite(PIN_LED, HIGH);  // LED indikator power ON

  lcd.init();
  Wire.setWireTimeout(3000, true);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Pengingat TRI SANDYA");
  lcd.setCursor(0, 1);
  lcd.print("Menyiapkan...");

  if (!rtc.begin()) {
    lcd.clear();
    lcd.print("RTC DS3231");
    lcd.setCursor(0, 1);
    lcd.print("TIDAK TERDETEKSI");
    Serial.println("RTC tidak ditemukan, cek wiring SDA/SCL!");
    while (1) delay(1000);
  }

  if (rtc.lostPower()) {
    // hanya set waktu default kalau RTC benar-benar kehilangan daya (baterai habis/baru)
    // waktu real tetap harus diatur manual lewat tombol kalibrasi
    rtc.adjust(DateTime(2021, 1, 1, 0, 0, 0));
    Serial.println("RTC kehilangan daya, waktu direset. Silakan kalibrasi via tombol MODE.");
  }

  // setup DFPlayer
  dfSerial.begin(9600);
  if (!mp3.begin(dfSerial)) {
    lcd.clear();
    lcd.print("DFPlayer");
    lcd.setCursor(0, 1);
    lcd.print("TIDAK TERDETEKSI");
    Serial.println("DFPlayer tidak ditemukan, cek wiring RX/TX & kartu SD!");
    while (1) delay(1000);
  }
  mp3.volume(MP3_VOLUME); // 0-30

  // tes relay sekali saat start (opsional, bisa dihapus)
  // digitalWrite(PIN_RELAY, HIGH);
  // delay(300);
  // digitalWrite(PIN_RELAY, LOW);

  lcd.clear();
}

void loop() {
  handleButtons();

  if (calibField == -1) {
    // ------------ MODE NORMAL: cek alarm & tampilkan jam ------------
    DateTime now = rtc.now();
    checkAlarms(now);
    updateRelayTimeout();

    if (millis() - lastLcdUpdate >= 500) {
      lastLcdUpdate = millis();
      tampilkanJamNormal(now);
    }
  } else {
    // ------------ MODE KALIBRASI: tampilkan editor ------------
    updateRelayTimeout(); // relay tetap dipantau walau lagi kalibrasi
    tampilkanKalibrasi();
  }
}

// =====================================================================
//                          FUNGSI TAMPILAN
// =====================================================================
void tampilkanJamNormal(DateTime &now) {
  lcd.setCursor(0, 0);
  char buf1[17];
  snprintf(buf1, sizeof(buf1), "%02d/%02d/%04d %02d:%02d",
           now.day(), now.month(), now.year(), now.hour(), now.minute());
  lcd.print(buf1);

  lcd.setCursor(0, 1);
  char buf2[17];
  snprintf(buf2, sizeof(buf2), "%-8s %s", namaHari[now.dayOfTheWeek()],
           relayActive ? "RELAY:1 " : "RELAY:0");
  lcd.print(buf2);
}

void tampilkanKalibrasi() {
  lcd.setCursor(0, 0);
  lcd.print("KALIBRASI RTC   ");
  lcd.setCursor(0, 1);

  char buf[17];
  switch (calibField) {
    case 0: snprintf(buf, sizeof(buf), "Tanggal: %02d     ", calibDay); break;
    case 1: snprintf(buf, sizeof(buf), "Bulan  : %02d     ", calibMonth); break;
    case 2: snprintf(buf, sizeof(buf), "Tahun  : %04d   ", calibYear); break;
    case 3: snprintf(buf, sizeof(buf), "Jam    : %02d     ", calibHour); break;
    case 4: snprintf(buf, sizeof(buf), "Menit  : %02d     ", calibMinute); break;
    case 5: snprintf(buf, sizeof(buf), "Detik  : %02d     ", calibSecond); break;
  }
  lcd.print(buf);
}

// =====================================================================
//                          FUNGSI ALARM & RELAY
// =====================================================================
void checkAlarms(DateTime &now) {
  for (uint8_t i = 0; i < 3; i++) {
    Alarm &a = alarms[i];
    if (now.hour() == a.hour && now.minute() == a.minute) {
      if (!a.triggeredToday) {
        a.triggeredToday = true;
        nyalakanRelay();
        delay(1000);
        mp3.play(a.track);
        Serial.print("Alarm ");
        Serial.print(a.label);
        Serial.println(" aktif, relay ON, mainkan track " + String(a.track));
      }
    } else if (now.minute() != a.minute) {
      // reset flag begitu menit sudah lewat, siap untuk hari berikutnya
      a.triggeredToday = false;
    }
  }
}

void nyalakanRelay() {
  digitalWrite(PIN_RELAY, HIGH);
  relayActive = true;
  relayOnAt = millis();
}

void updateRelayTimeout() {
  if (relayActive && (millis() - relayOnAt >= RELAY_ON_DURATION_MS)) {
    digitalWrite(PIN_RELAY, LOW);
    relayActive = false;
  }
}

// =====================================================================
//                          FUNGSI TOMBOL & KALIBRASI
// =====================================================================
void handleButtons() {
  if (millis() - lastBtnPress < DEBOUNCE_MS) return;

  bool mode = digitalRead(PIN_BTN_MODE) == LOW;
  bool up   = digitalRead(PIN_BTN_UP) == LOW;
  bool down = digitalRead(PIN_BTN_DOWN) == LOW;

  if (!mode && !up && !down) return;
  lastBtnPress = millis();

  if (mode) {
    if (calibField == -1) {
      // masuk mode kalibrasi, ambil waktu sekarang sebagai titik awal edit
      DateTime now = rtc.now();
      calibDay = now.day();
      calibMonth = now.month();
      calibYear = now.year();
      calibHour = now.hour();
      calibMinute = now.minute();
      calibSecond = now.second();
      calibField = 0;
    } else if (calibField < 5) {
      calibField++;
    } else {
      // sudah di field terakhir (Detik) -> simpan & keluar
      rtc.adjust(DateTime(calibYear, calibMonth, calibDay, calibHour, calibMinute, calibSecond));
      calibField = -1;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Waktu disimpan!");
      delay(1000);
      lcd.clear();
    }
    return;
  }

  if (calibField == -1) return; // UP/DOWN cuma aktif saat kalibrasi

  int dir = up ? 1 : (down ? -1 : 0);
  if (dir == 0) return;

  switch (calibField) {
    case 0: calibDay = wrapValue(calibDay + dir, 1, 31); break;
    case 1: calibMonth = wrapValue(calibMonth + dir, 1, 12); break;
    case 2: calibYear = wrapValue(calibYear + dir, 2020, 2099); break;
    case 3: calibHour = wrapValue(calibHour + dir, 0, 23); break;
    case 4: calibMinute = wrapValue(calibMinute + dir, 0, 59); break;
    case 5: calibSecond = wrapValue(calibSecond + dir, 0, 59); break;
  }
}

int wrapValue(int val, int minV, int maxV) {
  if (val > maxV) return minV;
  if (val < minV) return maxV;
  return val;
}
