/***********************************************************************************************
 * ConfigStorage - Configuration persistante sur LittleFS (JSON)
 *
 * RuntimeConfig : structure contenant tous les parametres modifiables a chaud.
 * Supporte un nombre variable de doigts (1-15) et de notes (1-32).
 * Initialisee avec les valeurs par defaut de settings.h, puis surchargee par
 * le fichier /config.json sur LittleFS.
 *
 * Permet de modifier les parametres via la page web sans recompiler.
 * Un appel a save() persiste les changements. load() les restaure au boot.
 *
 * Dependances : ArduinoJson, LittleFS
 ***********************************************************************************************/
#ifndef CONFIG_STORAGE_H
#define CONFIG_STORAGE_H

#include <Arduino.h>
#include "settings.h"

#define CONFIG_FILE_PATH "/config.json"

/*******************************************************************************
 * Structures pour doigts et notes (runtime, taille MAX)
 ******************************************************************************/

struct FingerConfig {
  uint8_t pcaChannel;      // Canal PCA9685 (0-15)
  uint16_t closedAngle;    // Angle position fermee (0-180)
  int8_t direction;        // +1 = horaire, -1 = anti-horaire
  bool isThumbHole;        // true = trou arriere (dessous de la flute)
};

struct NoteConfig {
  uint8_t midiNote;                          // Numero MIDI
  bool fingerPattern[MAX_FINGER_SERVOS];     // Doigtes (false=ferme, true=ouvert)
  uint8_t airflowMinPercent;                 // % min servo flow (0-100)
  uint8_t airflowMaxPercent;                 // % max servo flow (0-100)
};

/*******************************************************************************
 * RuntimeConfig - Configuration complete modifiable a chaud
 ******************************************************************************/

struct RuntimeConfig {
  // --- Instrument (modulaire) ---
  uint8_t numFingers;                        // 1-15 (nombre effectif de doigts)
  uint8_t numNotes;                          // 1-32 (nombre effectif de notes)
  uint8_t airflowPcaChannel;                // Canal PCA9685 pour servo airflow
  uint8_t fingerAngleOpen;                   // Amplitude ouverture commune (degres)
  FingerConfig fingers[MAX_FINGER_SERVOS];   // Config doigts (seuls [0..numFingers-1] actifs)
  NoteConfig notes[MAX_NOTES];               // Config notes (seuls [0..numNotes-1] actifs)

  // --- MIDI ---
  uint8_t midiChannel;

  // --- Timing ---
  uint16_t servoToSolenoidDelayMs;
  uint16_t minNoteIntervalForValveCloseMs;
  uint16_t minNoteDurationMs;

  // --- Airflow servo ---
  uint16_t servoAirflowOff;
  uint16_t servoAirflowMin;
  uint16_t servoAirflowMax;

  // --- Vibrato ---
  float vibratoFrequencyHz;
  float vibratoMaxAmplitudeDeg;

  // --- CC defaults ---
  uint8_t ccVolumeDefault;
  uint8_t ccExpressionDefault;
  uint8_t ccModulationDefault;
  uint8_t ccBreathDefault;
  uint8_t ccBrightnessDefault;

  // --- CC2 Breath Controller ---
  bool cc2Enabled;
  uint8_t cc2SilenceThreshold;
  float cc2ResponseCurve;
  uint16_t cc2TimeoutMs;

  // --- Solenoide ---
  uint8_t solenoidPwmActivation;
  uint8_t solenoidPwmHolding;
  uint16_t solenoidActivationTimeMs;

  // --- WiFi STA ---
  char wifiSsid[33];
  char wifiPassword[65];

  // --- Device ---
  char deviceName[32];

  // --- Power ---
  uint16_t timeUnpower;
};

// Config globale accessible depuis tout le projet
extern RuntimeConfig cfg;

/*******************************************************************************
 * Fonctions utilitaires runtime (remplacent les anciennes de settings.h)
 ******************************************************************************/

// Cherche une note par numero MIDI dans cfg.notes[]
inline const NoteConfig* getNoteByMidi(uint8_t midiNote) {
  for (int i = 0; i < cfg.numNotes; i++) {
    if (cfg.notes[i].midiNote == midiNote) {
      return &cfg.notes[i];
    }
  }
  return nullptr;
}

// Retourne l'index d'une note dans cfg.notes[], ou -1
inline int getNoteIndex(uint8_t midiNote) {
  for (int i = 0; i < cfg.numNotes; i++) {
    if (cfg.notes[i].midiNote == midiNote) {
      return i;
    }
  }
  return -1;
}

/*******************************************************************************
 * ConfigStorage - Classe statique de gestion persistance
 ******************************************************************************/

class ConfigStorage {
public:
  // Initialise cfg avec les valeurs par defaut de settings.h
  static void initDefaults();

  // Charge la config depuis LittleFS (surcharge les defauts)
  static bool load();

  // Sauvegarde la config actuelle sur LittleFS
  static bool save();

  // Remet cfg aux valeurs par defaut et sauvegarde
  static void resetToDefaults();
};

#endif
