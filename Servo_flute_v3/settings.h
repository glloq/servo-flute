/***********************************************************************************************
----------------------------         SETTINGS V3            ------------------------------------
Configuration pour servo-flute avec système binaire (ouvert/fermé uniquement)
Architecture avec servo débit + solénoïde valve
************************************************************************************************/
#ifndef SETTINGS_H
#define SETTINGS_H
#include "stdint.h"

#define DEBUG 1

/*******************************************************************************
-------------------------         MIDI SETTINGS        ------------------------
******************************************************************************/
#define FIRST_MIDI_NOTE 72  // Do5 (C5)

/*******************************************************************************
---------------------------   TIMING SETTINGS (ms)    ------------------------
******************************************************************************/
#define SERVO_SETTLE_TIME_MS    100   // Temps de déplacement des servos doigts
#define AIRFLOW_SETTLE_TIME_MS  20    // Temps de déplacement servo débit
#define STABILIZATION_TIME_MS   5     // Délai avant ouverture solénoïde
#define MIN_NOTE_DURATION_MS    10    // Durée minimale pour produire un son

/*******************************************************************************
---------------------------   EVENT QUEUE SETTINGS    ------------------------
******************************************************************************/
#define EVENT_QUEUE_SIZE 16  // Nombre max d'événements MIDI en buffer

/*******************************************************************************
---------------------------     SOLENOID VALVE        ------------------------
******************************************************************************/
#define SOLENOID_PIN 13           // Pin GPIO pour contrôle solénoïde
#define SOLENOID_ACTIVE_HIGH true // true = HIGH pour activer, false = LOW

/*******************************************************************************
---------------------------   AIR FLOW SERVO          ------------------------
******************************************************************************/
#define NUM_SERVO_AIRFLOW 10      // Canal PCA9685 pour servo débit air
#define SERVO_AIRFLOW_OFF 20      // Angle repos (pas de note)
#define SERVO_AIRFLOW_MIN 60      // Angle pour velocity=1 (pianissimo)
#define SERVO_AIRFLOW_MAX 100     // Angle pour velocity=127 (fortissimo)

/*******************************************************************************
---------------------------   FINGERS SERVOS          ------------------------
******************************************************************************/
#define TIMEUNPOWER 200               // Temps avant coupure alimentation servos (anti-bruit)
#define PIN_SERVOS_OFF 5              // Pin OE carte PCA9685 (0=off, 1=on)

#define NUMBER_SERVOS_FINGER 10       // Nombre de servos pour les doigts
#define NUMBER_NOTES 21               // Nombre de notes jouables (binaire seulement)
#define ANGLE_OPEN 30                 // Angle d'ouverture du trou (degrés)

// Angles de position fermée pour chaque servo (calibration individuelle)
const uint16_t closedAngles[NUMBER_SERVOS_FINGER] = {90, 100, 95, 100, 90, 95, 90, 90, 100, 90};

// Sens de rotation : 1 = horaire, -1 = anti-horaire
const int sensRotation[NUMBER_SERVOS_FINGER] = {-1, 1, 1, 1, 1, -1, -1, 1, 1, 1};

/******************************************************************************
---  FINGERING TABLE : BINARY ONLY (0=closed, 1=open) ---
21 notes jouables avec positions binaires uniquement
*/
const int finger_position[][NUMBER_SERVOS_FINGER] = {
  {0,0,0,0,0,0,0,0,0,0},  // Do5  => MIDI note 72
  {0,0,0,0,0,0,0,0,0,1},  // Do#5 => MIDI note 73
  {0,0,0,0,0,0,0,0,1,1},  // Ré5  => MIDI note 74
  {0,0,0,0,0,0,0,1,1,1},  // Ré#5 => MIDI note 75
  {0,0,0,0,0,0,1,1,1,1},  // Mi5  => MIDI note 76
  {0,0,0,0,0,1,1,1,1,1},  // Fa5  => MIDI note 77
  {0,0,0,0,0,1,0,0,0,0},  // Fa#5 => MIDI note 78
  {0,0,0,0,1,0,0,0,0,0},  // Sol5 => MIDI note 79
  {0,0,0,0,1,0,0,0,1,1},  // Sol#5=> MIDI note 80
  {0,0,0,0,1,1,1,1,1,1},  // La5  => MIDI note 81
  {0,0,0,1,0,0,0,1,1,1},  // La#5 => MIDI note 82
  {0,0,0,1,1,1,1,1,1,1},  // Si5  => MIDI note 83
  {0,0,1,0,0,1,1,1,1,1},  // Do6  => MIDI note 84
  {0,1,0,0,0,1,1,1,1,1},  // Do#6 => MIDI note 85
  {0,0,1,1,1,1,1,1,1,1},  // Ré6  => MIDI note 86
  {0,1,0,0,1,1,1,1,1,1},  // Ré#6 => MIDI note 87
  {0,1,0,1,1,1,1,1,1,1},  // Mi6  => MIDI note 88
  {1,0,0,1,1,1,1,1,1,1},  // Fa6  => MIDI note 89
  {0,1,1,1,1,1,1,1,1,1},  // Fa#6 => MIDI note 90
  {1,1,0,1,1,1,1,1,1,1},  // Sol6 => MIDI note 91
  {1,1,0,0,0,0,0,0,1,1}   // Sol#6=> MIDI note 92
};

/*******************************************************************************
-----------------------    SERVO PWM PARAMETERS       ------------------------
******************************************************************************/
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180
const uint16_t SERVO_PULSE_MIN = 550;   // Largeur impulsion min (µs)
const uint16_t SERVO_PULSE_MAX = 2450;  // Largeur impulsion max (µs)
const uint16_t SERVO_FREQUENCY = 50;    // Fréquence PWM (Hz)

#endif
