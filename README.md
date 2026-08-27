# ⚡ Arduino & IoT Project Archive

<div align="center">

![Arduino](https://img.shields.io/badge/Arduino-00979D?style=for-the-badge&logo=Arduino&logoColor=white)
![ESP32](https://img.shields.io/badge/ESP32-E7352C?style=for-the-badge&logo=Espressif&logoColor=white)
![ESP8266](https://img.shields.io/badge/ESP8266-black?style=for-the-badge&logo=Espressif&logoColor=white)
![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg?style=for-the-badge)

**Arsip terstruktur untuk berbagai proyek Embedded Systems, Robotic Arm, IoT Platform, dan Display Graphics.**

[📋 Daftar Proyek](#-katalog-proyek--showcase) • [⚡ CLI Helper (ard)](#-cli-helper-tool-ard) • [📁 Starter Templates](#-starter-templates) • [🚀 Cara Memulai](#-cara-memulai)

</div>

---

## 📖 Tentang Repositori

Repositori ini merupakan ruang kerja (*sketchbook archive*) yang mengorganisir berbagai eksperimen mikrokontroler berbasis **Arduino AVR (Uno/Nano)**, **ESP32**, dan **ESP8266 NodeMCU**. Setiap proyek dilengkapi struktur non-blocking, interpolasi gerak halus, hingga integrasi cloud IoT.

---

## 📋 Katalog Proyek & Showcase

### 🦾 Robotics & Motion Control
| Proyek | Target Board | Deskripsi |
|---|---|---|
| [`robot_arm_smooth/`](./robot_arm_smooth) | `Arduino Uno` | **6-DOF Robotic Arm** dengan 7x MG996R Servo (Root, Arm A1/A2, Arm B, Wrist A/B, Gripper). Dilengkapi interpolasi gerak halus (*smooth step-by-step*) dan sekuens otomatis **Pick & Place**. |
| [`servo_test/`](./servo_test) | `Arduino Uno` | Tool interaktif via Serial Monitor untuk kalibrasi sudut 0°, 90°, deteksi beban, dan *emergency stop/detach* motor MG996R. |
| [`vector_robot_project/`](./vector_robot_project) | `ESP32 Dev Module` | Robot interaktif dengan animasi mata OLED 128x64 dan kontrol pergerakan kepala 2-axis (*pan-tilt servos*). |
| [`motor_esp32/`](./motor_esp32) | `ESP32 Dev Module` | Pengendali dual motor DC menggunakan driver **L298N** berbasis sinyal PWM dari ESP32. |

---

### 🌐 Smart Home & Cloud IoT
| Proyek | Target Board | Deskripsi |
|---|---|---|
| [`my_home_iot_full/`](./my_home_iot_full) | `ESP8266 NodeMCU` | Hub Sensor Rumah Pintar: DHT11 (Suhu/Kelembaban), MQ Gas Sensor, PIR Motion Sensor, Buzzer, dan transmisi data aman via **HTTPS POST ke NexFlux IoT Cloud**. |
| [`smart-iot-desk/`](./smart-iot-desk) | `ESP32 Dev Module` | Dashboard meja pintar dengan visualisasi OLED 128x64, touch button, alarm buzzer, dan integrasi WiFi. |
| [`moisture_nexflux/`](./moisture_nexflux) | `ESP8266 NodeMCU` | Telemetri kelembaban tanah (*soil moisture*) untuk monitoring tanaman berkala via cloud API. |

---

### ⏰ Embedded Utilities & Display
| Proyek | Target Board | Deskripsi |
|---|---|---|
| [`pengingat_sembahyang_v2/`](./pengingat_sembahyang_v2) | `Arduino Uno` | Pengingat waktu otomatis dengan **RTC DS3231**, display LCD 16x2 I2C, modul suara **DFPlayer Mini MP3**, kontrol Relay, dan 3 tombol kalibrasi fisik. |
| [`pray_test_module/`](./pray_test_module) | `Arduino Uno` | Diagnostic test bed untuk integrasi RTC DS3231, LCD 16x2 I2C, dan modul relay. |
| [`oled_starboy/`](./oled_starboy) | `Arduino Uno` | Pemutar visual lirik terkoordinasi dan rendering custom bitmap grafis pada layar OLED SSD1306 128x64. |
| [`oled_temp/`](./oled_temp) | `ESP32 / Uno` | Monitor suhu & kelembaban real-time menggunakan sensor DHT11 pada display OLED I2C. |
| [`RTC_CALIBRADSI/`](./RTC_CALIBRADSI) | `Arduino Uno` | Utility sketch untuk sinkronisasi waktu awal chip RTC DS1307/DS3231 dengan waktu kompilasi komputer. |
| [`test_dht11/`](./test_dht11) | `ESP8266` | Skrip diagnosa cepat dan verifikasi pembacaan pin D4 sensor DHT11. |
| [`step_1/`](./step_1) | `Arduino Uno` | Serial command controller dasar untuk kontrol output relay/LED via terminal. |

---

## ⚡ CLI Helper Tool (`ard`)

Repositori ini dilengkapi script pembantu berbasis **`arduino-cli`** agar Anda dapat meng-compile, upload, dan memonitor sketch langsung dari Terminal tanpa harus membuka Arduino IDE GUI.

```bash
# 1. Menampilkan board USB yang terhubung
bash ./ard list

# 2. Compile sketch (Target Board FQBN otomatis terdeteksi)
bash ./ard compile robot_arm_smooth
bash ./ard compile my_home_iot_full

# 3. Upload firmware ke mikrokontroler
bash ./ard upload robot_arm_smooth /dev/cu.usbserial-110

# 4. Membuka Serial Monitor langsung di terminal
bash ./ard monitor /dev/cu.usbserial-110 9600

# 5. Batch compile test untuk memvalidasi SEMUA sketch di workspace
bash ./ard test-all
```

---

## 📁 Starter Templates (`_templates/`)

Untuk menjaga kode proyek baru tetap bersih dan terhindar dari file monolitik raksasa, gunakan template di folder [`_templates/`](./_templates):

- **[`avr_smooth_motion/`](./_templates/avr_smooth_motion)**: Template kontrol servo berbasis C++ Class (`MotionEngine`), memisahkan `config.h` (pinout & safety limits) dari logic utama.
- **[`esp32_iot_modular/`](./_templates/esp32_iot_modular)**: Template IoT ESP32 modular dengan manajemen WiFi non-blocking (auto-reconnect) dan template `secrets.example.h` yang aman dari kebocoran token.

---

## 🔌 Hardware Wiring Highlights

### 🤖 6-DOF Robot Arm Pinout (`robot_arm_smooth`)
```text
Arduino Uno Pinout:
 ├── Pin D3  ──> Servo 0: Root / Base Rotation
 ├── Pin D4  ──> Servo 1: Arm A1 (Shoulder)
 ├── Pin D5  ──> Servo 2: Arm A2 (Shoulder Mirror)
 ├── Pin D6  ──> Servo 3: Arm B (Elbow)
 ├── Pin D9  ──> Servo 4: Wrist A (Pitch)
 ├── Pin D10 ──> Servo 5: Wrist B (Roll)
 └── Pin D11 ──> Servo 6: Gripper
```
> [!IMPORTANT]
> **Power Supply**: Jangan mengambil daya 7x Servo MG996R dari pin 5V Arduino. Gunakan power supply eksternal (5V–6V 4A–5A) dengan **Common Ground** tersambung ke GND Arduino.

---

## 🚀 Cara Memulai

### 1. Prasyarat
- [Arduino IDE 2.x](https://www.arduino.cc/en/software) atau [Arduino CLI](https://arduino.github.io/arduino-cli/latest/)
- Package Core:
  - `arduino:avr` (Arduino Uno/Nano)
  - `esp32:esp32` (ESP32 Dev Module)
  - `esp8266:esp8266` (NodeMCU)

### 2. Clone Repositori
```bash
git clone https://github.com/joeinus134131/arduino-ide-archive.git
cd arduino-ide-archive
```

### 3. Keamanan Kredensial (Secrets)
Jika Anda menggunakan proyek IoT (`my_home_iot_full` / `smart-iot-desk`), salin template kredensial dan sesuaikan WiFi Anda:
```bash
# File secrets.h otomatis diabaikan oleh .gitignore agar tidak terunggah ke publik
cp _templates/esp32_iot_modular/secrets.example.h my_project/secrets.h
```

---

<div align="center">
  <sub>Dikelola dengan ❤️ untuk pengembangan embedded systems & IoT.</sub>
</div>
