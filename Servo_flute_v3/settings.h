/***********************************************************************************************
----------------------------         SETTINGS V3            ------------------------------------
Configuration centralisée pour servo-flute
Architecture avec servo débit + solénoïde valve + mode binaire (ouvert/fermé)
************************************************************************************************/
#ifndef SETTINGS_H
#define SETTINGS_H
#include "stdint.h"

#define DEBUG 1

/*******************************************************************************
-------------------------   CONFIGURATION INSTRUMENT  ------------------------
Modifier cette section pour adapter à un autre instrument (tin whistle, etc.)
******************************************************************************/

// Nombre de servos pour les doigts (6, 8, 10, 12...)
#define NUMBER_SERVOS_FINGER 10

// Nombre de notes jouables
#define NUMBER_NOTES 21

/*******************************************************************************
---------------------------   TIMING SETTINGS (ms)    ------------------------
******************************************************************************/
// Délai total entre positionnement servos et activation solénoïde
#define SERVO_TO_SOLENOID_DELAY_MS  105

// Si deux notes sont espacées de moins que ce délai, on garde la valve ouverte
#define MIN_NOTE_INTERVAL_FOR_VALVE_CLOSE_MS  50

#define MIN_NOTE_DURATION_MS    10

/*******************************************************************************
---------------------------   EVENT QUEUE SETTINGS    ------------------------
******************************************************************************/
#define EVENT_QUEUE_SIZE 16

/*******************************************************************************
---------------------------     SOLENOID VALVE        ------------------------
******************************************************************************/
#define SOLENOID_PIN 13
#define SOLENOID_ACTIVE_HIGH true

// MODE PWM (option pour réduction chaleur)
#define SOLENOID_USE_PWM true

// Paramètres PWM (si SOLENOID_USE_PWM = true)
#define SOLENOID_PWM_ACTIVATION 255
#define SOLENOID_PWM_HOLDING    128
#define SOLENOID_ACTIVATION_TIME_MS 50

/*******************************************************************************
---------------------------   AIR FLOW SERVO          ------------------------
******************************************************************************/
#define NUM_SERVO_AIRFLOW 10      // Canal PCA9685 pour servo débit air
#define SERVO_AIRFLOW_OFF 20      // Angle repos (pas de note)
#define SERVO_AIRFLOW_MIN 60      // Angle minimum absolu
#define SERVO_AIRFLOW_MAX 100     // Angle maximum absolu

/*******************************************************************************
---------------------------   POWER MANAGEMENT        ------------------------
******************************************************************************/
#define TIMEUNPOWER 200
#define PIN_SERVOS_OFF 5

/*******************************************************************************
------------------   CONFIGURATION SERVOS DOIGTS       ----------------------
Structure : {PCA_channel, angle_fermé, sens_ouverture}

PCA_channel     : Canal PCA9685 (0-15)
angle_fermé     : Angle servo en position fermée (0-180°)
sens_ouverture  : 1 = horaire, -1 = anti-horaire
******************************************************************************/
#define ANGLE_OPEN 30  // Angle d'ouverture du trou (degrés)

struct FingerConfig {
  byte pcaChannel;      // Canal PCA9685
  uint16_t closedAngle; // Angle position fermée
  int8_t direction;     // 1=horaire, -1=anti-horaire
};

const FingerConfig FINGERS[NUMBER_SERVOS_FINGER] = {
  // PCA  Fermé  Sens
  {  0,   90,   -1  },  // Doigt 0
  {  1,   100,   1  },  // Doigt 1
  {  2,   95,    1  },  // Doigt 2
  {  3,   100,   1  },  // Doigt 3
  {  4,   90,    1  },  // Doigt 4
  {  5,   95,   -1  },  // Doigt 5
  {  6,   90,   -1  },  // Doigt 6
  {  7,   90,    1  },  // Doigt 7
  {  8,   100,   1  },  // Doigt 8
  {  9,   90,    1  }   // Doigt 9
};

/*******************************************************************************
-----------------   CONFIGURATION DES NOTES JOUABLES   ----------------------
Structure : {MIDI, {doigtés}, flow_min%, flow_max%}

MIDI         : Numéro MIDI (72-127)
Doigtés      : Tableau (0=fermé, 1=ouvert)
flow_min%    : Pourcentage MIN ouverture servo flow (0-100%)
flow_max%    : Pourcentage MAX ouverture servo flow (0-100%)

Les pourcentages sont appliqués sur la plage [SERVO_AIRFLOW_MIN, SERVO_AIRFLOW_MAX]

EXEMPLE :
  {72, {0,0,0,0,0,0,0,0,0,0}, 0, 50}    // Do5: 0-50% de la plage
  {92, {1,1,0,0,0,0,0,0,1,1}, 40, 100}  // Sol#6: octave haute, 40-100%

→ Permet gestion volume ET adaptation débit selon octave
******************************************************************************/

struct NoteDefinition {
  byte midiNote;                            // Numéro MIDI
  bool fingerPattern[NUMBER_SERVOS_FINGER]; // Doigtés (0=fermé, 1=ouvert)
  byte airflowMinPercent;                   // % min servo flow (0-100)
  byte airflowMaxPercent;                   // % max servo flow (0-100)
};

// TABLE DES NOTES - Flûte à bec soprano
const NoteDefinition NOTES[NUMBER_NOTES] = {
  // MIDI  Doigtés (0=fermé, 1=ouvert)                    Min%  Max%
  {  72,  {0,0,0,0,0,0,0,0,0,0},  0,   50  },  // Do5  - Grave
  {  73,  {0,0,0,0,0,0,0,0,0,1},  0,   50  },  // Do#5
  {  74,  {0,0,0,0,0,0,0,0,1,1},  0,   50  },  // Ré5
  {  75,  {0,0,0,0,0,0,0,1,1,1},  0,   50  },  // Ré#5
  {  76,  {0,0,0,0,0,0,1,1,1,1},  0,   50  },  // Mi5
  {  77,  {0,0,0,0,0,1,1,1,1,1},  0,   50  },  // Fa5
  {  78,  {0,0,0,0,0,1,0,0,0,0},  0,   60  },  // Fa#5
  {  79,  {0,0,0,0,1,0,0,0,0,0},  0,   60  },  // Sol5
  {  80,  {0,0,0,0,1,0,0,0,1,1},  0,   60  },  // Sol#5
  {  81,  {0,0,0,0,1,1,1,1,1,1},  0,   60  },  // La5
  {  82,  {0,0,0,1,0,0,0,1,1,1},  0,   60  },  // La#5
  {  83,  {0,0,0,1,1,1,1,1,1,1},  0,   70  },  // Si5
  {  84,  {0,0,1,0,0,1,1,1,1,1},  20,  80  },  // Do6  - Médium, +air
  {  85,  {0,1,0,0,0,1,1,1,1,1},  20,  80  },  // Do#6
  {  86,  {0,0,1,1,1,1,1,1,1,1},  20,  80  },  // Ré6
  {  87,  {0,1,0,0,1,1,1,1,1,1},  20,  80  },  // Ré#6
  {  88,  {0,1,0,1,1,1,1,1,1,1},  30,  90  },  // Mi6
  {  89,  {1,0,0,1,1,1,1,1,1,1},  30,  90  },  // Fa6
  {  90,  {0,1,1,1,1,1,1,1,1,1},  30,  90  },  // Fa#6
  {  91,  {1,1,0,1,1,1,1,1,1,1},  40,  100 },  // Sol6 - Aigu, ++air
  {  92,  {1,1,0,0,0,0,0,0,1,1},  40,  100 }   // Sol#6
};

// Note MIDI la plus basse (calculée automatiquement)
#define FIRST_MIDI_NOTE (NOTES[0].midiNote)

// Fonction utilitaire pour obtenir une note par MIDI
inline const NoteDefinition* getNoteByMidi(byte midiNote) {
  for (int i = 0; i < NUMBER_NOTES; i++) {
    if (NOTES[i].midiNote == midiNote) {
      return &NOTES[i];
    }
  }
  return nullptr;
}

// Fonction utilitaire pour obtenir l'index d'une note
inline int getNoteIndex(byte midiNote) {
  for (int i = 0; i < NUMBER_NOTES; i++) {
    if (NOTES[i].midiNote == midiNote) {
      return i;
    }
  }
  return -1;
}

/*******************************************************************************
-----------------------    SERVO PWM PARAMETERS       ------------------------
******************************************************************************/
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180
const uint16_t SERVO_PULSE_MIN = 550;
const uint16_t SERVO_PULSE_MAX = 2450;
const uint16_t SERVO_FREQUENCY = 50;

#endif
