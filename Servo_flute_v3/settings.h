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
// = Temps déplacement servos + stabilisation mécanique
#define SERVO_TO_SOLENOID_DELAY_MS  105   // Temps total avant ouverture valve

// Si deux notes sont espacées de moins que ce délai, on garde la valve ouverte
// et on change juste les doigts (économie usure solénoïde + fluidité)
#define MIN_NOTE_INTERVAL_FOR_VALVE_CLOSE_MS  50

#define MIN_NOTE_DURATION_MS    10    // Durée minimale pour produire un son

/*******************************************************************************
---------------------------   EVENT QUEUE SETTINGS    ------------------------
******************************************************************************/
#define EVENT_QUEUE_SIZE 16  // Nombre max d'événements MIDI en buffer

/*******************************************************************************
---------------------------     SOLENOID VALVE        ------------------------
******************************************************************************/
#define SOLENOID_PIN 13           // Pin GPIO/PWM pour contrôle solénoïde
#define SOLENOID_ACTIVE_HIGH true // true = HIGH pour activer, false = LOW

// MODE PWM (option pour réduction chaleur)
#define SOLENOID_USE_PWM true     // true = PWM, false = GPIO simple (on/off)

// Paramètres PWM (si SOLENOID_USE_PWM = true)
#define SOLENOID_PWM_ACTIVATION 255    // PWM max pour ouverture rapide (0-255)
#define SOLENOID_PWM_HOLDING    128    // PWM réduit pour maintien (réduit chaleur)
#define SOLENOID_ACTIVATION_TIME_MS 50 // Temps PWM max avant réduction (ms)

/*******************************************************************************
---------------------------   AIR FLOW SERVO          ------------------------
******************************************************************************/
#define NUM_SERVO_AIRFLOW 10      // Canal PCA9685 pour servo débit air
#define SERVO_AIRFLOW_OFF 20      // Angle repos (pas de note)
#define SERVO_AIRFLOW_MIN 60      // Angle pour velocity=1 (pianissimo)
#define SERVO_AIRFLOW_MAX 100     // Angle pour velocity=127 (fortissimo)

/*******************************************************************************
---------------------------   POWER MANAGEMENT        ------------------------
******************************************************************************/
#define TIMEUNPOWER 200               // Temps avant coupure alimentation servos (anti-bruit)
#define PIN_SERVOS_OFF 5              // Pin OE carte PCA9685 (0=off, 1=on)

/*******************************************************************************
------------------   MAPPING SERVOS DOIGTS → PCA9685   ----------------------
Permet de définir l'ordre de branchement physique des servos
Index 0-9 = ordre logique des doigts (0=premier trou, 9=dernier trou)
Valeur = canal PCA9685 (0-15)

EXEMPLE pour inverser l'ordre:
  const int fingerToPCAChannel[10] = {9,8,7,6,5,4,3,2,1,0};
******************************************************************************/
const int fingerToPCAChannel[NUMBER_SERVOS_FINGER] = {
  0,  // Doigt 0 (1er trou) → PCA9685 canal 0
  1,  // Doigt 1 (2e trou)  → PCA9685 canal 1
  2,  // Doigt 2 (3e trou)  → PCA9685 canal 2
  3,  // Doigt 3 (4e trou)  → PCA9685 canal 3
  4,  // Doigt 4 (5e trou)  → PCA9685 canal 4
  5,  // Doigt 5 (6e trou)  → PCA9685 canal 5
  6,  // Doigt 6 (7e trou)  → PCA9685 canal 6
  7,  // Doigt 7 (8e trou)  → PCA9685 canal 7
  8,  // Doigt 8 (9e trou)  → PCA9685 canal 8
  9   // Doigt 9 (10e trou) → PCA9685 canal 9
};

/*******************************************************************************
------------------   CALIBRATION SERVOS DOIGTS         ----------------------
******************************************************************************/
#define ANGLE_OPEN 30  // Angle d'ouverture du trou (degrés)

// Angles de position fermée pour chaque doigt (calibration individuelle)
const uint16_t closedAngles[NUMBER_SERVOS_FINGER] = {
  90,   // Doigt 0 : angle fermé
  100,  // Doigt 1 : angle fermé
  95,   // Doigt 2 : angle fermé
  100,  // Doigt 3 : angle fermé
  90,   // Doigt 4 : angle fermé
  95,   // Doigt 5 : angle fermé
  90,   // Doigt 6 : angle fermé
  90,   // Doigt 7 : angle fermé
  100,  // Doigt 8 : angle fermé
  90    // Doigt 9 : angle fermé
};

// Sens de rotation : 1 = horaire, -1 = anti-horaire
const int sensRotation[NUMBER_SERVOS_FINGER] = {
  -1,  // Doigt 0
   1,  // Doigt 1
   1,  // Doigt 2
   1,  // Doigt 3
   1,  // Doigt 4
  -1,  // Doigt 5
  -1,  // Doigt 6
   1,  // Doigt 7
   1,  // Doigt 8
   1   // Doigt 9
};

/*******************************************************************************
-----------------   CONFIGURATION DES NOTES JOUABLES   ----------------------
Structure : Chaque ligne définit une note complète
Format : {MIDI, "Nom", {doigtés}, airflowMin}

MIDI        : Numéro MIDI de la note (72-127)
Nom         : Nom lisible (ex: "Do5", "C5", etc.)
Doigtés     : Tableau de 10 valeurs (0=fermé, 1=ouvert)
              Index correspond à l'ordre des doigts (pas forcément PCA channel!)
airflowMin  : Angle servo débit minimum pour cette note (optionnel)
              Si 0, utilise SERVO_AIRFLOW_MIN par défaut

EXEMPLE :
  {72, "Do5", {0,0,0,0,0,0,0,0,0,0}, 0},  // Tous fermés, airflow défaut
  {73, "Re5", {0,0,0,0,0,0,0,0,0,1}, 65}, // Dernier ouvert, airflow custom
******************************************************************************/

// Structure pour définir une note
struct NoteDefinition {
  byte midiNote;                            // Numéro MIDI
  const char* name;                         // Nom lisible
  bool fingerPattern[NUMBER_SERVOS_FINGER]; // Doigtés (0=fermé, 1=ouvert)
  byte minAirflow;                          // Angle servo débit min (0=défaut)
};

// TABLE DES NOTES - Flûte à bec soprano
const NoteDefinition NOTES[NUMBER_NOTES] = {
  // MIDI  Nom      Doigtés (0=fermé, 1=ouvert)                    Airflow
  {  72,  "Do5",  {0,0,0,0,0,0,0,0,0,0},  0  },  // Do5  - Tous fermés
  {  73,  "Do#5", {0,0,0,0,0,0,0,0,0,1},  0  },  // Do#5
  {  74,  "Re5",  {0,0,0,0,0,0,0,0,1,1},  0  },  // Ré5
  {  75,  "Re#5", {0,0,0,0,0,0,0,1,1,1},  0  },  // Ré#5
  {  76,  "Mi5",  {0,0,0,0,0,0,1,1,1,1},  0  },  // Mi5
  {  77,  "Fa5",  {0,0,0,0,0,1,1,1,1,1},  0  },  // Fa5
  {  78,  "Fa#5", {0,0,0,0,0,1,0,0,0,0},  0  },  // Fa#5
  {  79,  "Sol5", {0,0,0,0,1,0,0,0,0,0},  0  },  // Sol5
  {  80,  "Sol#5",{0,0,0,0,1,0,0,0,1,1},  0  },  // Sol#5
  {  81,  "La5",  {0,0,0,0,1,1,1,1,1,1},  0  },  // La5
  {  82,  "La#5", {0,0,0,1,0,0,0,1,1,1},  0  },  // La#5
  {  83,  "Si5",  {0,0,0,1,1,1,1,1,1,1},  0  },  // Si5
  {  84,  "Do6",  {0,0,1,0,0,1,1,1,1,1},  0  },  // Do6
  {  85,  "Do#6", {0,1,0,0,0,1,1,1,1,1},  0  },  // Do#6
  {  86,  "Re6",  {0,0,1,1,1,1,1,1,1,1},  0  },  // Ré6
  {  87,  "Re#6", {0,1,0,0,1,1,1,1,1,1},  0  },  // Ré#6
  {  88,  "Mi6",  {0,1,0,1,1,1,1,1,1,1},  0  },  // Mi6
  {  89,  "Fa6",  {1,0,0,1,1,1,1,1,1,1},  0  },  // Fa6
  {  90,  "Fa#6", {0,1,1,1,1,1,1,1,1,1},  0  },  // Fa#6
  {  91,  "Sol6", {1,1,0,1,1,1,1,1,1,1},  0  },  // Sol6
  {  92,  "Sol#6",{1,1,0,0,0,0,0,0,1,1},  0  }   // Sol#6
};

// Note MIDI la plus basse (calculée automatiquement)
#define FIRST_MIDI_NOTE (NOTES[0].midiNote)

// Fonction utilitaire pour obtenir une note par index
inline const NoteDefinition* getNoteByMidi(byte midiNote) {
  for (int i = 0; i < NUMBER_NOTES; i++) {
    if (NOTES[i].midiNote == midiNote) {
      return &NOTES[i];
    }
  }
  return nullptr;
}

// Fonction utilitaire pour obtenir l'index d'une note MIDI
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
const uint16_t SERVO_PULSE_MIN = 550;   // Largeur impulsion min (µs)
const uint16_t SERVO_PULSE_MAX = 2450;  // Largeur impulsion max (µs)
const uint16_t SERVO_FREQUENCY = 50;    // Fréquence PWM (Hz)

#endif
