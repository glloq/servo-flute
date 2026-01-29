/***********************************************************************************************
 * CALIBRATION MANAGER
 *
 * Chef d'orchestre du processus de calibration.
 * Gère le menu principal et coordonne les différents calibrateurs.
 ***********************************************************************************************/
#ifndef CALIBRATION_MANAGER_H
#define CALIBRATION_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "settings_template.h"
#include "FingerCalibrator.h"
#include "AirflowCalibrator.h"
#include "OutputGenerator.h"

class CalibrationManager {
public:
  CalibrationManager();

  // Initialisation
  void begin();

  // Boucle principale
  void run();

private:
  Adafruit_PWMServoDriver _pwm;
  FingerCalibrator _fingerCal;
  AirflowCalibrator _airflowCal;
  OutputGenerator _outputGen;

  // Stockage temporaire des calibrations
  FingerConfig _calibratedFingers[NUMBER_SERVOS_FINGER];
  NoteDefinition _calibratedNotes[NUMBER_NOTES];

  // État de calibration
  bool _fingersCalibrated;
  bool _notesCalibrated;

  // Menu principal
  void displayMainMenu();
  void handleMenuChoice(int choice);

  // Modes de calibration
  void calibrateAllFingers();
  void calibrateAllNotes();
  void displayCurrentConfig();
  void generateOutput();

  // Utilitaires
  void initializeDefaults();
  int waitForMenuChoice();
  void printWelcomeBanner();
  void printCalibrationStatus();
};

#endif
