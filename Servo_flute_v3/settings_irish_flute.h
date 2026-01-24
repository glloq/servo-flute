/***********************************************************************************************
----------------------------         SETTINGS V3            ------------------------------------
Configuration centralisée pour FLÛTE IRLANDAISE (Irish Flute en D)
Architecture avec servo débit + solénoïde valve + mode binaire (ouvert/fermé)
************************************************************************************************/
#ifndef SETTINGS_H
#define SETTINGS_H
#include "stdint.h"

#define DEBUG 1

/*******************************************************************************
-------------------------   CONFIGURATION INSTRUMENT  ------------------------
FLÛTE IRLANDAISE EN C (Irish Flute / Tin Whistle)
- 6 trous
- Tonalité: C majeur (Do majeur)
- Gamme: A#5 (La#5) à C7 (Do7)
******************************************************************************/

// Nombre de servos pour les doigts
#define NUMBER_SERVOS_FINGER 6

// Nombre de notes jouables
#define NUMBER_NOTES 14

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

ORDRE DES TROUS (Irish Flute standard) :
  0 = Trou 1 (haut, main gauche - index)
  1 = Trou 2 (main gauche - majeur)
  2 = Trou 3 (main gauche - annulaire)
  3 = Trou 4 (main droite - index)
  4 = Trou 5 (main droite - majeur)
  5 = Trou 6 (bas, main droite - annulaire)
******************************************************************************/
#define ANGLE_OPEN 30  // Angle d'ouverture du trou (degrés)

struct FingerConfig {
  byte pcaChannel;      // Canal PCA9685
  uint16_t closedAngle; // Angle position fermée
  int8_t direction;     // 1=horaire, -1=anti-horaire
};

const FingerConfig FINGERS[NUMBER_SERVOS_FINGER] = {
  // PCA  Fermé  Sens
  {  0,   90,   -1  },  // Trou 1 (haut)
  {  1,   95,    1  },  // Trou 2
  {  2,   90,    1  },  // Trou 3
  {  3,   100,   1  },  // Trou 4
  {  4,   95,   -1  },  // Trou 5
  {  5,   90,    1  }   // Trou 6 (bas)
};

/*******************************************************************************
-----------------   CONFIGURATION DES NOTES JOUABLES   ----------------------
Structure : {MIDI, {doigtés}, flow_min%, flow_max%}

MIDI         : Numéro MIDI (82-96 pour A#5-C7)
Doigtés      : Tableau 6 trous (0=fermé, 1=ouvert)
flow_min%    : Pourcentage MIN ouverture servo flow (0-100%)
flow_max%    : Pourcentage MAX ouverture servo flow (0-100%)

Les pourcentages sont appliqués sur la plage [SERVO_AIRFLOW_MIN, SERVO_AIRFLOW_MAX]

DOIGTÉS IRISH FLUTE EN C :
  000000 = C (note de base selon octave)
  000001 = D
  000011 = E
  000111 = F
  001111 = G
  011111 = A
  111111 = B

  Octaves : même doigtés avec plus d'air
******************************************************************************/

struct NoteDefinition {
  byte midiNote;                            // Numéro MIDI
  bool fingerPattern[NUMBER_SERVOS_FINGER]; // Doigtés (0=fermé, 1=ouvert)
  byte airflowMinPercent;                   // % min servo flow (0-100)
  byte airflowMaxPercent;                   // % max servo flow (0-100)
};

// TABLE DES NOTES - Flûte irlandaise en C (à partir de A#5)
// LOGIQUE PHYSIQUE : Plus de trous fermés = colonne d'air longue = PLUS d'air
//                    Plus de trous ouverts = colonne d'air courte = MOINS d'air
const NoteDefinition NOTES[NUMBER_NOTES] = {
  // OCTAVE BASSE - Notes graves (A#5 à B5)
  // MIDI  Doigtés (6 trous)  Min%  Max%
  {  82,  {0,1,1,1,1,1},  10,  60  },  // A#5 (La#5) - 1 fermé
  {  83,  {1,1,1,1,1,1},  0,   50  },  // B5  (Si5) - Tous ouverts, moins d'air

  // OCTAVE 1 - MÉDIUM (C6 à B6)
  {  84,  {0,0,0,0,0,0},  20,  75  },  // C6  (Do6) - Tous fermés, PLUS d'air
  {  86,  {0,0,0,0,0,1},  15,  70  },  // D6  (Ré6) - 5 fermés
  {  88,  {0,0,0,0,1,1},  10,  65  },  // E6  (Mi6) - 4 fermés
  {  89,  {0,0,0,1,1,1},  10,  60  },  // F6  (Fa6) - 3 fermés
  {  91,  {0,0,1,1,1,1},  5,   55  },  // G6  (Sol6) - 2 fermés
  {  93,  {0,1,1,1,1,1},  5,   50  },  // A6  (La6) - 1 fermé
  {  95,  {1,1,1,1,1,1},  0,   45  },  // B6  (Si6) - Tous ouverts, MOINS d'air

  // OCTAVE 2 - AIGU (C7 à G7) - Octave sup = plus d'air partout
  {  96,  {0,0,0,0,0,0},  50,  100 },  // C7  (Do7) - Tous fermés, octave haute
  {  98,  {0,0,0,0,0,1},  45,  95  },  // D7  (Ré7)
  {  100, {0,0,0,0,1,1},  40,  90  },  // E7  (Mi7)
  {  101, {0,0,0,1,1,1},  35,  85  },  // F7  (Fa7)
  {  103, {0,0,1,1,1,1},  30,  80  }   // G7  (Sol7) - Moins fermé, moins d'air
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
