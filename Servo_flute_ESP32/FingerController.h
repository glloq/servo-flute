#ifndef FINGER_CONTROLLER_H
#define FINGER_CONTROLLER_H

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include "settings.h"

class FingerController {
public:
  FingerController(Adafruit_PWMServoDriver& pwm);

  void begin();

  // Applique un pattern de doigtes binaire (false=ferme, true=ouvert)
  void setFingerPattern(const bool pattern[NUMBER_SERVOS_FINGER]);

  // Applique un pattern pour une note MIDI specifique
  void setFingerPatternForNote(byte midiNote);

  void closeAllFingers();
  void openAllFingers();

private:
  Adafruit_PWMServoDriver& _pwm;

  uint16_t calculateServoAngle(int fingerIndex, bool isOpen);
  void setServoAngle(int fingerIndex, uint16_t angle);
  uint16_t angleToPWM(uint16_t angle);
};

#endif
