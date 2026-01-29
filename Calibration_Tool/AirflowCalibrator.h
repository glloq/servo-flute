/***********************************************************************************************
 * AIRFLOW CALIBRATOR
 *
 * Gère la calibration des plages airflow pour chaque note (min% et max%).
 ***********************************************************************************************/
#ifndef AIRFLOW_CALIBRATOR_H
#define AIRFLOW_CALIBRATOR_H

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include "settings_template.h"

class AirflowCalibrator {
public:
  AirflowCalibrator(Adafruit_PWMServoDriver& pwm);

  // Calibre une note complète (airflowMin% et airflowMax%)
  void calibrateNote(int noteIndex,
                     const FingerConfig calibratedFingers[],
                     NoteDefinition& output);

private:
  Adafruit_PWMServoDriver& _pwm;

  // État interne
  int _currentNoteIndex;
  byte _currentPercent;
  int _step;  // 1=min%, 2=max%

  // Appliquer le doigté de la note
  void applyFingering(const bool fingerPattern[], const FingerConfig calibratedFingers[]);

  // Calibrer airflowMinPercent
  void calibrateMinPercent();

  // Calibrer airflowMaxPercent
  void calibrateMaxPercent(byte minPercent);

  // Vérification finale
  bool verifyConfiguration(const NoteDefinition& note);

  // Utilitaires
  void setAirflowPercent(byte percent);
  uint16_t percentToAngle(byte percent);
  void openSolenoid();
  void closeSolenoid();
  void adjustPercent(int delta);
  void testNote(int durationMs);
  void setServoAngle(byte pcaChannel, uint16_t angle);
  uint16_t angleToPWM(uint16_t angle);
  bool waitForConfirmation();
};

#endif
