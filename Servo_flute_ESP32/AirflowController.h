#ifndef AIRFLOW_CONTROLLER_H
#define AIRFLOW_CONTROLLER_H

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include "settings.h"

class AirflowController {
public:
  AirflowController(Adafruit_PWMServoDriver& pwm);

  void begin();

  // Definit le debit d'air selon la velocite MIDI (1-127)
  void setAirflowVelocity(byte velocity);

  // Definit le debit d'air pour une note specifique avec velocite
  void setAirflowForNote(byte midiNote, byte velocity);

  void openSolenoid();
  void closeSolenoid();
  bool isSolenoidOpen() const;
  void setAirflowToRest();

  // Methode update pour gestion PWM solenoide et vibrato (appeler dans loop)
  void update();

  // Met a jour les valeurs CC (appele par InstrumentManager)
  void setCCValues(byte ccVolume, byte ccExpression, byte ccModulation);

  // Met a jour CC2 (Breath Controller) avec lissage et fallback velocity
  void updateCC2Breath(byte ccBreath);

  // Calibration : positionner le servo airflow a un angle arbitraire
  void testAirflowAngle(uint16_t angle);
  // Calibration : tester solenoide open/close
  void testSolenoid(bool open);

private:
  Adafruit_PWMServoDriver& _pwm;
  bool _solenoidOpen;
  unsigned long _solenoidOpenTime;

  // Valeurs Control Change MIDI
  byte _ccVolume;
  byte _ccExpression;
  byte _ccModulation;
  byte _ccBreath;

  // Breath Controller (CC2)
  byte _cc2SmoothingBuffer[CC2_SMOOTHING_BUFFER_SIZE];
  uint8_t _cc2BufferIndex;
  uint8_t _cc2BufferCount;
  unsigned long _lastCC2Time;
  byte _lastVelocity;

  // Gestion vibrato
  uint16_t _baseAngleWithoutVibrato;
  bool _vibratoActive;
  uint16_t _currentMinAngle;
  uint16_t _currentMaxAngle;

  void setAirflowServoAngle(uint16_t angle);
  uint16_t angleToPWM(uint16_t angle);
  void setSolenoidPWM(uint8_t pwmValue);
};

#endif
