/***********************************************************************************************
 * CALIBRATION TOOL - SERVO FLUTE V3
 *
 * Outil de calibration pour les servos doigts et plages airflow.
 *
 * Instructions:
 * 1. Téléverser ce sketch sur l'Arduino
 * 2. Ouvrir le Serial Monitor (115200 baud)
 * 3. Suivre le menu pour calibrer:
 *    - D'abord les servos doigts (angles fermés + sens)
 *    - Puis les plages airflow par note (min% et max%)
 * 4. Générer le code settings.h final
 * 5. Copier-coller le code dans Servo_flute_v3/settings.h
 *
 * Hardware requis:
 * - Arduino Leonardo/Micro
 * - PCA9685 PWM Driver (I2C)
 * - Servos montés sur l'instrument
 * - Solénoïde connecté
 * - Alimentation air fonctionnelle
 *
 * Version: 1.0 MVP
 * Date: 2026-01-29
 ***********************************************************************************************/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "settings_template.h"
#include "CalibrationManager.h"

// Instance du gestionnaire de calibration
CalibrationManager calibManager;

void setup() {
  // Initialiser le gestionnaire
  calibManager.begin();
}

void loop() {
  // Exécuter le menu principal
  calibManager.run();
}
