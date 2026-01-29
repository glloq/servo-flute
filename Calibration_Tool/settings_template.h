/***********************************************************************************************
 * CALIBRATION TOOL - SETTINGS TEMPLATE
 *
 * Ce fichier définit la configuration de base pour la calibration.
 * Modifiez uniquement les valeurs de base (nombre de doigts, notes, canaux PCA).
 * Les valeurs calibrées (angles, %) seront générées automatiquement.
 ***********************************************************************************************/
#ifndef SETTINGS_TEMPLATE_H
#define SETTINGS_TEMPLATE_H

#include <Arduino.h>

/*******************************************************************************
-------------------------   CONFIGURATION INSTRUMENT  ------------------------
******************************************************************************/
// Nombre de servos pour les doigts
#define NUMBER_SERVOS_FINGER 6

// Nombre de notes jouables
#define NUMBER_NOTES 14

/*******************************************************************************
---------------------------   HARDWARE SETTINGS       ------------------------
******************************************************************************/
// Solénoïde
#define SOLENOID_PIN 13
#define SOLENOID_ACTIVE_HIGH true

// Servo airflow
#define NUM_SERVO_AIRFLOW 10      // Canal PCA9685 pour servo débit air
#define SERVO_AIRFLOW_OFF 20      // Angle repos (pas de note)
#define SERVO_AIRFLOW_MIN 60      // Angle minimum absolu
#define SERVO_AIRFLOW_MAX 100     // Angle maximum absolu

// Servos doigts
#define ANGLE_OPEN 30             // Angle d'ouverture du trou (degrés)

// Servo PWM
#define SERVO_PULSE_MIN 550
#define SERVO_PULSE_MAX 2450
#define SERVO_FREQUENCY 50

/*******************************************************************************
------------------   TEMPLATE SERVOS DOIGTS (À CALIBRER)  --------------------
******************************************************************************/
struct FingerConfig {
  byte pcaChannel;      // Canal PCA9685
  uint16_t closedAngle; // Angle position fermée (À CALIBRER)
  int8_t direction;     // 1=horaire, -1=anti-horaire (À CALIBRER)
};

// TEMPLATE - Uniquement les canaux PCA sont fixes
// closedAngle et direction seront calibrés
const FingerConfig FINGERS_TEMPLATE[NUMBER_SERVOS_FINGER] = {
  // PCA  Fermé  Sens
  {  0,   90,    1  },  // Trou 1 (haut, main gauche - index)
  {  1,   90,    1  },  // Trou 2 (main gauche - majeur)
  {  2,   90,    1  },  // Trou 3 (main gauche - annulaire)
  {  3,   90,    1  },  // Trou 4 (main droite - index)
  {  4,   90,    1  },  // Trou 5 (main droite - majeur)
  {  5,   90,    1  }   // Trou 6 (bas, main droite - annulaire)
};

/*******************************************************************************
-------------   TEMPLATE NOTES JOUABLES (DOIGTÉS FIXES)  --------------------
******************************************************************************/
struct NoteDefinition {
  byte midiNote;                            // Numéro MIDI
  bool fingerPattern[NUMBER_SERVOS_FINGER]; // Doigtés (0=fermé, 1=ouvert)
  byte airflowMinPercent;                   // % min servo flow (À CALIBRER)
  byte airflowMaxPercent;                   // % max servo flow (À CALIBRER)
};

// TEMPLATE - Les doigtés sont théoriques (Irish Flute standard)
// airflowMinPercent et airflowMaxPercent seront calibrés
const NoteDefinition NOTES_TEMPLATE[NUMBER_NOTES] = {
  // MIDI  Doigtés (6 trous)  Min%  Max%
  {  82,  {0,1,1,1,1,1},  10,  60  },  // A#5 (La#5) - 1 fermé
  {  83,  {1,1,1,1,1,1},  0,   50  },  // B5  (Si5) - Tous ouverts
  {  84,  {0,0,0,0,0,0},  20,  75  },  // C6  (Do6) - Tous fermés
  {  86,  {0,0,0,0,0,1},  15,  70  },  // D6  (Ré6) - 5 fermés
  {  88,  {0,0,0,0,1,1},  10,  65  },  // E6  (Mi6) - 4 fermés
  {  89,  {0,0,0,1,1,1},  10,  60  },  // F6  (Fa6) - 3 fermés
  {  91,  {0,0,1,1,1,1},  5,   55  },  // G6  (Sol6) - 2 fermés
  {  93,  {0,1,1,1,1,1},  5,   50  },  // A6  (La6) - 1 fermé
  {  95,  {1,1,1,1,1,1},  0,   45  },  // B6  (Si6) - Tous ouverts
  {  96,  {0,0,0,0,0,0},  50,  100 },  // C7  (Do7) - Tous fermés, octave haute
  {  98,  {0,0,0,0,0,1},  45,  95  },  // D7  (Ré7)
  {  100, {0,0,0,0,1,1},  40,  90  },  // E7  (Mi7)
  {  101, {0,0,0,1,1,1},  35,  85  },  // F7  (Fa7)
  {  103, {0,0,1,1,1,1},  30,  80  }   // G7  (Sol7)
};

// Noms des notes (pour affichage)
const char* NOTE_NAMES[NUMBER_NOTES] = {
  "A#5", "B5", "C6", "D6", "E6", "F6", "G6", "A6", "B6", "C7", "D7", "E7", "F7", "G7"
};

#endif
