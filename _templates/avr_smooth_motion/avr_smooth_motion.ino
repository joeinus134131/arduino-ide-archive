// ==============================================================================
//  Template: AVR Smooth Motion Controller (Modular Architecture)
//  Target: Arduino Uno / Nano (arduino:avr:uno)
// ==============================================================================

#include "config.h"
#include "motion_engine.h"

MotionEngine motion;
unsigned long lastLogTime = 0;

void setup() {
  Serial.begin(SERIAL_BAUD);
  motion.init();

  Serial.println(F("========================================"));
  Serial.println(F("   AVR Smooth Motion Engine Started     "));
  Serial.println(F("========================================"));
}

void loop() {
  // 1. Update pergerakan servo non-blocking
  motion.update();

  // 2. Logging status secara berkala
  unsigned long now = millis();
  if (now - lastLogTime >= LOG_INTERVAL) {
    lastLogTime = now;
    if (!motion.isTargetReached()) {
      Serial.print(F("Moving... [0]: "));
      Serial.print(motion.getAngle(0));
      Serial.print(F(" -> "));
      Serial.println(motion.getTarget(0));
    }
  }
}
