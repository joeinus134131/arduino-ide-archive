# 📂 Katalog Proyek Arduino Workspace

Dokumentasi terpusat untuk seluruh proyek sketch di workspace `/Users/user/Documents/Arduino`.

---

## 📋 Tabel Proyek & Target Hardware

| Folder Proyek | Target Board / FQBN | Deskripsi & Komponen Utama | Status |
|---|---|---|---|
| [`robot_arm_smooth`](file:///Users/user/Documents/Arduino/robot_arm_smooth/robot_arm_smooth.ino) | `arduino:avr:uno` | 6-DOF Robotic Arm (7x MG996R Servos: Pin 3, 4, 5, 6, 9, 10, 11) + Interpolasi halus & Pick & Place | 🟢 Aktif |
| [`servo_test`](file:///Users/user/Documents/Arduino/servo_test/servo_test.ino) | `arduino:avr:uno` | Tool Kalibrasi & Jog Test 1-by-1 Servo MG996R via Serial Monitor | 🛠️ Tool |
| [`pengingat_sembahyang_v2`](file:///Users/user/Documents/Arduino/pengingat_sembahyang_v2/pengingat_sembahyang_v2.ino) | `arduino:avr:uno` | Pengingat Sembahyang: RTC DS3231, LCD 16x2 I2C, DFPlayer Mini, Relay, 3 Tombol Kalibrasi | 🟢 Aktif |
| [`pray_test_module`](file:///Users/user/Documents/Arduino/pray_test_module/pray_test_module.ino) | `arduino:avr:uno` | Unit Test LCD 16x2 + RTC DS3231 + Relay | 🧪 Testing |
| [`my_home_iot_full`](file:///Users/user/Documents/Arduino/my_home_iot_full/my_home_iot_full.ino) | `esp8266:esp8266:nodemcuv2` | Smart Home Hub: DHT11 (D4), MQ Gas (A0), PIR (D6), Buzzer (D5) -> Push HTTPS NexFlux IoT | 🟢 Aktif |
| [`smart-iot-desk`](file:///Users/user/Documents/Arduino/smart-iot-desk/smart-iot-desk.ino) | `esp32:esp32:esp32da` | Smart IoT Desk: OLED 128x64 (SDA 8, SCL 9), DHT11 (D2), Touch (D3), Buzzer (D4), WiFi | 🟡 In Progress |
| [`vector_robot_project`](file:///Users/user/Documents/Arduino/vector_robot_project/vector_robot_project.ino) | `esp32:esp32:esp32da` | Vector Robot: Animasi Mata OLED 128x64 + 2x Pan-Tilt Servos | 🟢 Aktif |
| [`motor_esp32`](file:///Users/user/Documents/Arduino/motor_esp32/motor_esp32.ino) | `esp32:esp32:esp32da` | Dual DC Motor Controller L298N via ESP32 PWM | 🟢 Aktif |
| [`moisture_nexflux`](file:///Users/user/Documents/Arduino/moisture_nexflux/moisture_nexflux.ino) | `esp8266:esp8266:nodemcuv2` | Soil Moisture Sensor Telemetry ke NexFlux Cloud | 🟢 Aktif |
| [`oled_temp`](file:///Users/user/Documents/Arduino/oled_temp/oled_temp.ino) | `esp32:esp32:esp32da` / `avr:uno` | Monitor Suhu & Kelembaban DHT11 pada Layar OLED SSD1306 | 🛠️ Tool |
| [`oled_starboy`](file:///Users/user/Documents/Arduino/oled_starboy/sketch_may15a.ino) | `arduino:avr:uno` | Animasi Bitmap & Lyric Sync Player pada Layar OLED I2C | 🧪 Eksperimental |
| [`RTC_CALIBRADSI`](file:///Users/user/Documents/Arduino/RTC_CALIBRADSI/RTC_CALIBRADSI.ino) | `arduino:avr:uno` | Kalibrasi & Set Waktu RTC DS1307/DS3231 | 🛠️ Tool |
| [`test_dht11`](file:///Users/user/Documents/Arduino/test_dht11/test_dht11.ino) | `esp8266:esp8266:nodemcuv2` | Standalone Diagnostic Test Sensor DHT11 pada Pin D4 | 🧪 Testing |
| [`step_1`](file:///Users/user/Documents/Arduino/step_1/step_1.ino) | `arduino:avr:uno` | Basic Serial Controller LED On/Off | 📚 Edukasi |

---

## ⚡ CLI Shortcuts (`./ard`)

Gunakan script helper `./ard` untuk kompilasi dan upload cepat tanpa harus membuka Arduino IDE GUI:

```bash
# 1. Cek port board USB yang terhubung
./ard list

# 2. Compile sketch (Board FQBN otomatis terdeteksi)
./ard compile robot_arm_smooth
./ard compile my_home_iot_full

# 3. Upload sketch ke board
./ard upload robot_arm_smooth /dev/cu.usbserial-110

# 4. Buka Serial Monitor langsung di Terminal
./ard monitor /dev/cu.usbserial-110 9600

# 5. Validasi dan compile SEMUA proyek untuk mendeteksi error
./ard test-all
```

---

## 🛡️ Best Practices & Aturan Workspace

1. **Struktur Folder Sketch**:
   - Arduino mengharuskan nama file utama `.ino` sama persis dengan nama folder induknya (misal: `robot_arm_smooth/robot_arm_smooth.ino`).
2. **Kredensial Sensitif**:
   - Jangan menuliskan password WiFi atau token IoT langsung di file `.ino`.
   - Gunakan file `secrets.h` (sudah diabaikan oleh `.gitignore`).
3. **Power Supply Servo**:
   - Untuk proyek dengan servo banyak (seperti `robot_arm_smooth`), selalu gunakan power supply eksternal 5V/6V (minimal 3A–5A) dengan **Common Ground (GND eksternal terhubung ke GND Arduino)**.
