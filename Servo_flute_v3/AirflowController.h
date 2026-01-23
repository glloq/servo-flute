#ifndef AIRFLOW_CONTROLLER_H
#define AIRFLOW_CONTROLLER_H

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include "settings.h"

class AirflowController {
public:
  AirflowController(Adafruit_PWMServoDriver& pwm);

  // Initialise le servo débit et le solénoïde
  void begin();

  // Définit le débit d'air selon la vélocité MIDI (1-127)
  void setAirflowVelocity(byte velocity);

  // Ouvre le solénoïde (permet circulation d'air)
  void openSolenoid();

  // Ferme le solénoïde (bloque circulation d'air)
  void closeSolenoid();

  // Retourne l'état actuel du solénoïde
  bool isSolenoidOpen() const;

  // Positionne le servo débit en position repos
  void setAirflowToRest();

private:
  Adafruit_PWMServoDriver& _pwm;
  bool _solenoidOpen;

  // Positionne le servo de débit à un angle spécifique
  void setAirflowServoAngle(uint16_t angle);

  // Convertit un angle en valeur PWM pour le PCA9685
  uint16_t angleToPWM(uint16_t angle);
};

#endif
