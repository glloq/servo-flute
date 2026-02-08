#include "ConfigStorage.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

// Instance globale
RuntimeConfig cfg;

void ConfigStorage::initDefaults() {
  cfg.midiChannel = MIDI_CHANNEL;

  cfg.servoToSolenoidDelayMs = SERVO_TO_SOLENOID_DELAY_MS;
  cfg.minNoteIntervalForValveCloseMs = MIN_NOTE_INTERVAL_FOR_VALVE_CLOSE_MS;
  cfg.minNoteDurationMs = MIN_NOTE_DURATION_MS;

  cfg.servoAirflowOff = SERVO_AIRFLOW_OFF;
  cfg.servoAirflowMin = SERVO_AIRFLOW_MIN;
  cfg.servoAirflowMax = SERVO_AIRFLOW_MAX;

  cfg.vibratoFrequencyHz = VIBRATO_FREQUENCY_HZ;
  cfg.vibratoMaxAmplitudeDeg = VIBRATO_MAX_AMPLITUDE_DEG;

  cfg.ccVolumeDefault = CC_VOLUME_DEFAULT;
  cfg.ccExpressionDefault = CC_EXPRESSION_DEFAULT;
  cfg.ccModulationDefault = CC_MODULATION_DEFAULT;
  cfg.ccBreathDefault = CC_BREATH_DEFAULT;
  cfg.ccBrightnessDefault = CC_BRIGHTNESS_DEFAULT;

  cfg.cc2Enabled = CC2_ENABLED;
  cfg.cc2SilenceThreshold = CC2_SILENCE_THRESHOLD;
  cfg.cc2ResponseCurve = CC2_RESPONSE_CURVE;
  cfg.cc2TimeoutMs = CC2_TIMEOUT_MS;

  cfg.solenoidPwmActivation = SOLENOID_PWM_ACTIVATION;
  cfg.solenoidPwmHolding = SOLENOID_PWM_HOLDING;
  cfg.solenoidActivationTimeMs = SOLENOID_ACTIVATION_TIME_MS;

  cfg.fingerAngleOpen = ANGLE_OPEN;
  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    cfg.fingerClosedAngle[i] = FINGERS[i].closedAngle;
    cfg.fingerDirection[i] = FINGERS[i].direction;
  }

  for (int i = 0; i < NUMBER_NOTES; i++) {
    cfg.noteAirflowMin[i] = NOTES[i].airflowMinPercent;
    cfg.noteAirflowMax[i] = NOTES[i].airflowMaxPercent;
  }

  memset(cfg.wifiSsid, 0, sizeof(cfg.wifiSsid));
  memset(cfg.wifiPassword, 0, sizeof(cfg.wifiPassword));

  strncpy(cfg.deviceName, DEVICE_NAME, sizeof(cfg.deviceName) - 1);
  cfg.deviceName[sizeof(cfg.deviceName) - 1] = '\0';

  cfg.timeUnpower = TIMEUNPOWER;
}

bool ConfigStorage::load() {
  // D'abord initialiser les defauts
  initDefaults();

  File file = LittleFS.open(CONFIG_FILE_PATH, "r");
  if (!file) {
    if (DEBUG) {
      Serial.println("DEBUG: ConfigStorage - Pas de config sauvegardee, utilisation des defauts");
    }
    return false;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();

  if (err) {
    if (DEBUG) {
      Serial.print("ERREUR: ConfigStorage - JSON invalide: ");
      Serial.println(err.c_str());
    }
    return false;
  }

  // Surcharger chaque champ (l'operateur | fournit la valeur par defaut si absent)
  cfg.midiChannel = doc["midi_ch"] | cfg.midiChannel;

  cfg.servoToSolenoidDelayMs = doc["servo_delay"] | cfg.servoToSolenoidDelayMs;
  cfg.minNoteIntervalForValveCloseMs = doc["valve_interval"] | cfg.minNoteIntervalForValveCloseMs;
  cfg.minNoteDurationMs = doc["min_note_dur"] | cfg.minNoteDurationMs;

  cfg.servoAirflowOff = doc["air_off"] | cfg.servoAirflowOff;
  cfg.servoAirflowMin = doc["air_min"] | cfg.servoAirflowMin;
  cfg.servoAirflowMax = doc["air_max"] | cfg.servoAirflowMax;

  cfg.vibratoFrequencyHz = doc["vib_freq"] | cfg.vibratoFrequencyHz;
  cfg.vibratoMaxAmplitudeDeg = doc["vib_amp"] | cfg.vibratoMaxAmplitudeDeg;

  cfg.ccVolumeDefault = doc["cc_vol"] | cfg.ccVolumeDefault;
  cfg.ccExpressionDefault = doc["cc_expr"] | cfg.ccExpressionDefault;
  cfg.ccModulationDefault = doc["cc_mod"] | cfg.ccModulationDefault;
  cfg.ccBreathDefault = doc["cc_breath"] | cfg.ccBreathDefault;
  cfg.ccBrightnessDefault = doc["cc_bright"] | cfg.ccBrightnessDefault;

  cfg.cc2Enabled = doc["cc2_on"] | (cfg.cc2Enabled ? 1 : 0);
  cfg.cc2SilenceThreshold = doc["cc2_thr"] | cfg.cc2SilenceThreshold;
  cfg.cc2ResponseCurve = doc["cc2_curve"] | cfg.cc2ResponseCurve;
  cfg.cc2TimeoutMs = doc["cc2_timeout"] | cfg.cc2TimeoutMs;

  cfg.solenoidPwmActivation = doc["sol_act"] | cfg.solenoidPwmActivation;
  cfg.solenoidPwmHolding = doc["sol_hold"] | cfg.solenoidPwmHolding;
  cfg.solenoidActivationTimeMs = doc["sol_time"] | cfg.solenoidActivationTimeMs;

  cfg.fingerAngleOpen = doc["angle_open"] | cfg.fingerAngleOpen;

  // Fingers
  JsonArray fingers = doc["fingers"];
  if (fingers) {
    for (int i = 0; i < NUMBER_SERVOS_FINGER && i < (int)fingers.size(); i++) {
      cfg.fingerClosedAngle[i] = fingers[i]["a"] | cfg.fingerClosedAngle[i];
      cfg.fingerDirection[i] = fingers[i]["d"] | cfg.fingerDirection[i];
    }
  }

  // Notes airflow
  JsonArray notes = doc["notes_air"];
  if (notes) {
    for (int i = 0; i < NUMBER_NOTES && i < (int)notes.size(); i++) {
      cfg.noteAirflowMin[i] = notes[i]["mn"] | cfg.noteAirflowMin[i];
      cfg.noteAirflowMax[i] = notes[i]["mx"] | cfg.noteAirflowMax[i];
    }
  }

  const char* ssid = doc["wifi_ssid"];
  if (ssid) strncpy(cfg.wifiSsid, ssid, sizeof(cfg.wifiSsid) - 1);

  const char* pass = doc["wifi_pass"];
  if (pass) strncpy(cfg.wifiPassword, pass, sizeof(cfg.wifiPassword) - 1);

  const char* name = doc["device"];
  if (name) strncpy(cfg.deviceName, name, sizeof(cfg.deviceName) - 1);

  cfg.timeUnpower = doc["time_unpower"] | cfg.timeUnpower;

  if (DEBUG) {
    Serial.println("DEBUG: ConfigStorage - Config chargee depuis LittleFS");
    Serial.print("DEBUG:   MIDI ch: ");
    Serial.println(cfg.midiChannel);
    Serial.print("DEBUG:   WiFi SSID: ");
    Serial.println(strlen(cfg.wifiSsid) > 0 ? cfg.wifiSsid : "(vide)");
  }

  return true;
}

bool ConfigStorage::save() {
  JsonDocument doc;

  doc["midi_ch"] = cfg.midiChannel;

  doc["servo_delay"] = cfg.servoToSolenoidDelayMs;
  doc["valve_interval"] = cfg.minNoteIntervalForValveCloseMs;
  doc["min_note_dur"] = cfg.minNoteDurationMs;

  doc["air_off"] = cfg.servoAirflowOff;
  doc["air_min"] = cfg.servoAirflowMin;
  doc["air_max"] = cfg.servoAirflowMax;

  doc["vib_freq"] = cfg.vibratoFrequencyHz;
  doc["vib_amp"] = cfg.vibratoMaxAmplitudeDeg;

  doc["cc_vol"] = cfg.ccVolumeDefault;
  doc["cc_expr"] = cfg.ccExpressionDefault;
  doc["cc_mod"] = cfg.ccModulationDefault;
  doc["cc_breath"] = cfg.ccBreathDefault;
  doc["cc_bright"] = cfg.ccBrightnessDefault;

  doc["cc2_on"] = cfg.cc2Enabled ? 1 : 0;
  doc["cc2_thr"] = cfg.cc2SilenceThreshold;
  doc["cc2_curve"] = cfg.cc2ResponseCurve;
  doc["cc2_timeout"] = cfg.cc2TimeoutMs;

  doc["sol_act"] = cfg.solenoidPwmActivation;
  doc["sol_hold"] = cfg.solenoidPwmHolding;
  doc["sol_time"] = cfg.solenoidActivationTimeMs;

  doc["angle_open"] = cfg.fingerAngleOpen;

  JsonArray fingers = doc["fingers"].to<JsonArray>();
  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    JsonObject f = fingers.add<JsonObject>();
    f["a"] = cfg.fingerClosedAngle[i];
    f["d"] = cfg.fingerDirection[i];
  }

  JsonArray notes = doc["notes_air"].to<JsonArray>();
  for (int i = 0; i < NUMBER_NOTES; i++) {
    JsonObject n = notes.add<JsonObject>();
    n["mn"] = cfg.noteAirflowMin[i];
    n["mx"] = cfg.noteAirflowMax[i];
  }

  doc["wifi_ssid"] = cfg.wifiSsid;
  doc["wifi_pass"] = cfg.wifiPassword;
  doc["device"] = cfg.deviceName;
  doc["time_unpower"] = cfg.timeUnpower;

  File file = LittleFS.open(CONFIG_FILE_PATH, "w");
  if (!file) {
    if (DEBUG) {
      Serial.println("ERREUR: ConfigStorage - Impossible d'ecrire le fichier config");
    }
    return false;
  }

  size_t written = serializeJson(doc, file);
  file.close();

  if (DEBUG) {
    Serial.print("DEBUG: ConfigStorage - Config sauvegardee (");
    Serial.print(written);
    Serial.println(" octets)");
  }

  return written > 0;
}

void ConfigStorage::resetToDefaults() {
  initDefaults();
  save();
  if (DEBUG) {
    Serial.println("DEBUG: ConfigStorage - Reset aux valeurs par defaut");
  }
}
