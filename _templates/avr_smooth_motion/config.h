#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// PIN DEFINITIONS & SERVO MAPPING
// ==========================================
const int NUM_SERVOS = 6;
const int SERVO_PINS[NUM_SERVOS] = { 3, 4, 5, 6, 9, 10 };
const String SERVO_NAMES[NUM_SERVOS] = { "Base", "Shoulder", "Elbow", "Wrist Pitch", "Wrist Roll", "Gripper" };

// ==========================================
// SAFETY LIMITS (Min / Max Angle per servo)
// ==========================================
const int SERVO_MIN[NUM_SERVOS] = { 0,   10,  10,   0,   0,  20 };
const int SERVO_MAX[NUM_SERVOS] = { 180, 170, 170, 180, 180, 140 };

// ==========================================
// MOTION PARAMETERS
// ==========================================
const int DEFAULT_STEP_SIZE = 1;         // Degrees per step (1 = smoothest)
const unsigned long STEP_INTERVAL = 15;  // Milliseconds between steps

// ==========================================
// SERIAL BAUD RATE & LOGGING
// ==========================================
const long SERIAL_BAUD = 9600;
const unsigned long LOG_INTERVAL = 500;  // ms between status logs

#endif // CONFIG_H
