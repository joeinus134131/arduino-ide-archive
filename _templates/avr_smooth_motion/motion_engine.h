#ifndef MOTION_ENGINE_H
#define MOTION_ENGINE_H

#include <Arduino.h>
#include <Servo.h>
#include "config.h"

class MotionEngine {
private:
  Servo servos[NUM_SERVOS];
  int currentAngle[NUM_SERVOS];
  int targetAngle[NUM_SERVOS];
  unsigned long lastStepTime;

public:
  MotionEngine() : lastStepTime(0) {}

  void init() {
    for (int i = 0; i < NUM_SERVOS; i++) {
      currentAngle[i] = 90;
      targetAngle[i] = 90;
      servos[i].attach(SERVO_PINS[i]);
      servos[i].write(currentAngle[i]);
    }
  }

  void setTarget(int servoIdx, int angle) {
    if (servoIdx >= 0 && servoIdx < NUM_SERVOS) {
      targetAngle[servoIdx] = constrain(angle, SERVO_MIN[servoIdx], SERVO_MAX[servoIdx]);
    }
  }

  void setAllTargets(const int targets[NUM_SERVOS]) {
    for (int i = 0; i < NUM_SERVOS; i++) {
      setTarget(i, targets[i]);
    }
  }

  bool isTargetReached() const {
    for (int i = 0; i < NUM_SERVOS; i++) {
      if (currentAngle[i] != targetAngle[i]) {
        return false;
      }
    }
    return true;
  }

  void update() {
    unsigned long now = millis();
    if (now - lastStepTime < STEP_INTERVAL) return;
    lastStepTime = now;

    for (int i = 0; i < NUM_SERVOS; i++) {
      if (currentAngle[i] < targetAngle[i]) {
        currentAngle[i] = min(currentAngle[i] + DEFAULT_STEP_SIZE, targetAngle[i]);
        servos[i].write(currentAngle[i]);
      } else if (currentAngle[i] > targetAngle[i]) {
        currentAngle[i] = max(currentAngle[i] - DEFAULT_STEP_SIZE, targetAngle[i]);
        servos[i].write(currentAngle[i]);
      }
    }
  }

  int getAngle(int servoIdx) const {
    if (servoIdx >= 0 && servoIdx < NUM_SERVOS) return currentAngle[servoIdx];
    return 0;
  }

  int getTarget(int servoIdx) const {
    if (servoIdx >= 0 && servoIdx < NUM_SERVOS) return targetAngle[servoIdx];
    return 0;
  }
};

#endif // MOTION_ENGINE_H
