/***********************************************************************************************
 * FINGER CALIBRATOR
 *
 * Gère la calibration des servos doigts (angle fermé + sens de rotation).
 ***********************************************************************************************/
#ifndef FINGER_CALIBRATOR_H
#define FINGER_CALIBRATOR_H

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include "settings_template.h"

class FingerCalibrator {
public:
  FingerCalibrator(Adafruit_PWMServoDriver& pwm);

  // Calibre un doigt complet (angle fermé + sens)
  void calibrateFinger(int fingerIndex, FingerConfig& output);

private:
  Adafruit_PWMServoDriver& _pwm;

  // État interne
  int _currentFingerIndex;
  uint16_t _currentAngle;
  int8_t _currentDirection;
  int _step;  // 1=angle fermé, 2=sens, 3=vérification

  // Étape 1 : Calibrer angle fermé
  void calibrateClosedAngle(byte pcaChannel);

  // Étape 2 : Déterminer sens de rotation
  void calibrateDirection(byte pcaChannel, uint16_t closedAngle);

  // Étape 3 : Vérification finale
  bool verifyConfiguration(byte pcaChannel, uint16_t closedAngle, int8_t direction);

  // Utilitaires
  void setServoAngle(byte pcaChannel, uint16_t angle);
  uint16_t angleToPWM(uint16_t angle);
  void adjustAngle(int delta);
  void testCurrentPosition(byte pcaChannel);
  void testOpenClose(byte pcaChannel, uint16_t closedAngle, int8_t direction);
  char waitForCommand();
  bool waitForConfirmation();
};

#endif
