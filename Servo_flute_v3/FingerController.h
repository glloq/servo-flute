#ifndef FINGER_CONTROLLER_H
#define FINGER_CONTROLLER_H

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include "settings.h"

class FingerController {
public:
  FingerController(Adafruit_PWMServoDriver& pwm);

  // Initialise les servos en position fermée
  void begin();

  // Applique un pattern de doigtés binaire (false=fermé, true=ouvert)
  void setFingerPattern(const bool pattern[NUMBER_SERVOS_FINGER]);

  // Applique un pattern pour une note MIDI spécifique
  void setFingerPatternForNote(byte midiNote);

  // Ferme tous les doigts
  void closeAllFingers();

  // Ouvre tous les doigts
  void openAllFingers();

private:
  Adafruit_PWMServoDriver& _pwm;

  // Calcule l'angle pour un servo donné selon son état (false=fermé, true=ouvert)
  uint16_t calculateServoAngle(int fingerIndex, bool isOpen);

  // Envoie une commande d'angle à un servo spécifique
  void setServoAngle(int servoIndex, uint16_t angle);

  // Convertit un angle en valeur PWM pour le PCA9685
  uint16_t angleToPWM(uint16_t angle);
};

#endif
