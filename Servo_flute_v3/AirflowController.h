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

  // Définit le débit d'air pour une note spécifique avec vélocité
  void setAirflowForNote(byte midiNote, byte velocity);

  // Ouvre le solénoïde (permet circulation d'air)
  void openSolenoid();

  // Ferme le solénoïde (bloque circulation d'air)
  void closeSolenoid();

  // Retourne l'état actuel du solénoïde
  bool isSolenoidOpen() const;

  // Positionne le servo débit en position repos
  void setAirflowToRest();

  // Méthode update pour gestion PWM solénoïde (appeler dans loop)
  void update();

  // Met à jour les valeurs CC (appelé par InstrumentManager)
  void setCCValues(byte ccVolume, byte ccExpression, byte ccModulation);

  // Met à jour CC2 (Breath Controller) avec lissage et fallback velocity
  void updateCC2Breath(byte ccBreath);

private:
  Adafruit_PWMServoDriver& _pwm;
  bool _solenoidOpen;
  unsigned long _solenoidOpenTime;  // Timestamp ouverture solénoïde (pour PWM)

  // Valeurs Control Change MIDI
  byte _ccVolume;       // CC 7  (multiplicateur global)
  byte _ccExpression;   // CC 11 (expression dynamique)
  byte _ccModulation;   // CC 1  (vibrato)
  byte _ccBreath;       // CC 2  (breath controller)

  // Breath Controller (CC2) - Lissage et fallback
  byte _cc2SmoothingBuffer[CC2_SMOOTHING_BUFFER_SIZE];  // Buffer circulaire pour moyenne glissante
  uint8_t _cc2BufferIndex;                              // Index actuel dans buffer circulaire
  uint8_t _cc2BufferCount;                              // Nombre valeurs dans buffer (0-CC2_SMOOTHING_BUFFER_SIZE)
  unsigned long _lastCC2Time;                           // Timestamp dernier CC2 reçu (pour timeout fallback)
  byte _lastVelocity;                                   // Velocity stockée pour fallback si CC2 absent

  // Gestion vibrato
  uint16_t _baseAngleWithoutVibrato;  // Angle calculé sans vibrato (pour update continu)
  bool _vibratoActive;                 // True si vibrato doit être appliqué
  uint16_t _currentMinAngle;           // Angle minimum de la note en cours
  uint16_t _currentMaxAngle;           // Angle maximum de la note en cours

  // Positionne le servo de débit à un angle spécifique
  void setAirflowServoAngle(uint16_t angle);

  // Convertit un angle en valeur PWM pour le PCA9685
  uint16_t angleToPWM(uint16_t angle);

  // Contrôle solénoïde via GPIO ou PWM
  void setSolenoidPWM(uint8_t pwmValue);
};

#endif
