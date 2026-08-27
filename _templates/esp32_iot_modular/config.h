#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// HARDWARE PIN DEFINITIONS
// ==========================================
#define STATUS_LED_PIN   2
#define SENSOR_PIN       4
#define RELAY_PIN        5

// ==========================================
// TASK & TELEMETRY TIMING (ms)
// ==========================================
const unsigned long TELEMETRY_INTERVAL = 5000; // 5 seconds
const unsigned long WIFI_CHECK_INTERVAL = 10000; // 10 seconds

// ==========================================
// SERIAL BAUD RATE
// ==========================================
const long SERIAL_BAUD = 115200;

#endif // CONFIG_H
