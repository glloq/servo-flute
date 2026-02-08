/***********************************************************************************************
 * ConfigStorage - Configuration persistante sur LittleFS (JSON)
 *
 * RuntimeConfig : structure contenant tous les parametres modifiables a chaud.
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

struct RuntimeConfig {
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

  // --- Calibration doigts ---
  uint16_t fingerClosedAngle[NUMBER_SERVOS_FINGER];
  int8_t fingerDirection[NUMBER_SERVOS_FINGER];
  uint8_t fingerAngleOpen;

  // --- Airflow par note (%) ---
  uint8_t noteAirflowMin[NUMBER_NOTES];
  uint8_t noteAirflowMax[NUMBER_NOTES];

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
