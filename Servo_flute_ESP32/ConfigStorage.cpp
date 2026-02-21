#include "ConfigStorage.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

// Instance globale
RuntimeConfig cfg;

void ConfigStorage::initDefaults() {
  // --- Instrument ---
  cfg.numFingers = DEFAULT_NUM_FINGERS;
  cfg.numNotes = DEFAULT_NUM_NOTES;
  cfg.airflowPcaChannel = DEFAULT_AIRFLOW_PCA_CHANNEL;
  cfg.fingerAngleOpen = ANGLE_OPEN;

  // Zero-fill all arrays first
  memset(cfg.fingers, 0, sizeof(cfg.fingers));
  memset(cfg.notes, 0, sizeof(cfg.notes));

  // Load default finger configs
  for (int i = 0; i < DEFAULT_NUM_FINGERS && i < MAX_FINGER_SERVOS; i++) {
    cfg.fingers[i].pcaChannel = DEFAULT_FINGERS[i].pcaChannel;
    cfg.fingers[i].closedAngle = DEFAULT_FINGERS[i].closedAngle;
    cfg.fingers[i].direction = DEFAULT_FINGERS[i].direction;
    cfg.fingers[i].isThumbHole = DEFAULT_FINGERS[i].isThumbHole;
  }

  // Load default note configs
  for (int i = 0; i < DEFAULT_NUM_NOTES && i < MAX_NOTES; i++) {
    cfg.notes[i].midiNote = DEFAULT_NOTES[i].midiNote;
    cfg.notes[i].airflowMinPercent = DEFAULT_NOTES[i].airflowMinPercent;
    cfg.notes[i].airflowMaxPercent = DEFAULT_NOTES[i].airflowMaxPercent;
    // Copy finger pattern (default has DEFAULT_NUM_FINGERS, pad rest with 0)
    for (int f = 0; f < MAX_FINGER_SERVOS; f++) {
      cfg.notes[i].fingerPattern[f] = (f < DEFAULT_NUM_FINGERS) ? DEFAULT_NOTES[i].fingerPattern[f] : false;
    }
  }

  // --- MIDI ---
  cfg.midiChannel = MIDI_CHANNEL;

  // --- Timing ---
  cfg.servoToSolenoidDelayMs = SERVO_TO_SOLENOID_DELAY_MS;
  cfg.minNoteIntervalForValveCloseMs = MIN_NOTE_INTERVAL_FOR_VALVE_CLOSE_MS;
  cfg.minNoteDurationMs = MIN_NOTE_DURATION_MS;

  // --- Airflow ---
  cfg.servoAirflowOff = SERVO_AIRFLOW_OFF;
  cfg.servoAirflowMin = SERVO_AIRFLOW_MIN;
  cfg.servoAirflowMax = SERVO_AIRFLOW_MAX;

  // --- Vibrato ---
  cfg.vibratoFrequencyHz = VIBRATO_FREQUENCY_HZ;
  cfg.vibratoMaxAmplitudeDeg = VIBRATO_MAX_AMPLITUDE_DEG;

  // --- CC defaults ---
  cfg.ccVolumeDefault = CC_VOLUME_DEFAULT;
  cfg.ccExpressionDefault = CC_EXPRESSION_DEFAULT;
  cfg.ccModulationDefault = CC_MODULATION_DEFAULT;
  cfg.ccBreathDefault = CC_BREATH_DEFAULT;
  cfg.ccBrightnessDefault = CC_BRIGHTNESS_DEFAULT;

  // --- CC2 ---
  cfg.cc2Enabled = CC2_ENABLED;
  cfg.cc2SilenceThreshold = CC2_SILENCE_THRESHOLD;
  cfg.cc2ResponseCurve = CC2_RESPONSE_CURVE;
  cfg.cc2TimeoutMs = CC2_TIMEOUT_MS;

  // --- Solenoide ---
  cfg.solenoidPwmActivation = SOLENOID_PWM_ACTIVATION;
  cfg.solenoidPwmHolding = SOLENOID_PWM_HOLDING;
  cfg.solenoidActivationTimeMs = SOLENOID_ACTIVATION_TIME_MS;

  // --- WiFi ---
  memset(cfg.wifiSsid, 0, sizeof(cfg.wifiSsid));
  memset(cfg.wifiPassword, 0, sizeof(cfg.wifiPassword));

  // --- Device ---
  strncpy(cfg.deviceName, DEVICE_NAME, sizeof(cfg.deviceName) - 1);
  cfg.deviceName[sizeof(cfg.deviceName) - 1] = '\0';

  // --- Power ---
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

  // --- Instrument ---
  cfg.numFingers = doc["num_fingers"] | cfg.numFingers;
  if (cfg.numFingers < 1) cfg.numFingers = 1;
  if (cfg.numFingers > MAX_FINGER_SERVOS) cfg.numFingers = MAX_FINGER_SERVOS;

  cfg.numNotes = doc["num_notes"] | cfg.numNotes;
  if (cfg.numNotes > MAX_NOTES) cfg.numNotes = MAX_NOTES;

  cfg.airflowPcaChannel = doc["air_pca"] | cfg.airflowPcaChannel;
  cfg.fingerAngleOpen = doc["angle_open"] | cfg.fingerAngleOpen;

  // --- Fingers ---
  JsonArray fingers = doc["fingers"];
  if (fingers) {
    for (int i = 0; i < cfg.numFingers && i < (int)fingers.size(); i++) {
      JsonObject f = fingers[i];
      cfg.fingers[i].pcaChannel = f["ch"] | cfg.fingers[i].pcaChannel;
      cfg.fingers[i].closedAngle = f["a"] | cfg.fingers[i].closedAngle;
      cfg.fingers[i].direction = f["d"] | cfg.fingers[i].direction;
      cfg.fingers[i].isThumbHole = f["th"] | (cfg.fingers[i].isThumbHole ? 1 : 0);
    }
  }

  // --- Notes (complete: MIDI + finger patterns + airflow) ---
  JsonArray notes = doc["notes"];
  if (notes) {
    int count = min((int)notes.size(), (int)MAX_NOTES);
    cfg.numNotes = count;
    for (int i = 0; i < count; i++) {
      JsonObject n = notes[i];
      cfg.notes[i].midiNote = n["midi"] | cfg.notes[i].midiNote;
      cfg.notes[i].airflowMinPercent = n["amn"] | cfg.notes[i].airflowMinPercent;
      cfg.notes[i].airflowMaxPercent = n["amx"] | cfg.notes[i].airflowMaxPercent;
      JsonArray fp = n["fp"];
      if (fp) {
        for (int f = 0; f < MAX_FINGER_SERVOS; f++) {
          cfg.notes[i].fingerPattern[f] = (f < (int)fp.size()) ? (fp[f].as<int>() != 0) : false;
        }
      }
    }
  }

  // --- Scalaires (surcharge partielle, chaque champ optionnel) ---
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
  cfg.timeUnpower = doc["time_unpower"] | cfg.timeUnpower;

  const char* ssid = doc["wifi_ssid"];
  if (ssid) strncpy(cfg.wifiSsid, ssid, sizeof(cfg.wifiSsid) - 1);

  const char* pass = doc["wifi_pass"];
  if (pass) strncpy(cfg.wifiPassword, pass, sizeof(cfg.wifiPassword) - 1);

  const char* name = doc["device"];
  if (name) strncpy(cfg.deviceName, name, sizeof(cfg.deviceName) - 1);

  if (DEBUG) {
    Serial.println("DEBUG: ConfigStorage - Config chargee depuis LittleFS");
    Serial.print("DEBUG:   Doigts: ");
    Serial.print(cfg.numFingers);
    Serial.print("  Notes: ");
    Serial.print(cfg.numNotes);
    Serial.print("  Airflow PCA: ");
    Serial.println(cfg.airflowPcaChannel);
  }

  return true;
}

bool ConfigStorage::save() {
  JsonDocument doc;

  // --- Instrument ---
  doc["num_fingers"] = cfg.numFingers;
  doc["num_notes"] = cfg.numNotes;
  doc["air_pca"] = cfg.airflowPcaChannel;
  doc["angle_open"] = cfg.fingerAngleOpen;

  // --- Fingers ---
  JsonArray fingers = doc["fingers"].to<JsonArray>();
  for (int i = 0; i < cfg.numFingers; i++) {
    JsonObject f = fingers.add<JsonObject>();
    f["ch"] = cfg.fingers[i].pcaChannel;
    f["a"] = cfg.fingers[i].closedAngle;
    f["d"] = cfg.fingers[i].direction;
    if (cfg.fingers[i].isThumbHole) {
      f["th"] = 1;
    }
  }

  // --- Notes (complete) ---
  JsonArray notes = doc["notes"].to<JsonArray>();
  for (int i = 0; i < cfg.numNotes; i++) {
    JsonObject n = notes.add<JsonObject>();
    n["midi"] = cfg.notes[i].midiNote;
    n["amn"] = cfg.notes[i].airflowMinPercent;
    n["amx"] = cfg.notes[i].airflowMaxPercent;
    JsonArray fp = n["fp"].to<JsonArray>();
    for (int f = 0; f < cfg.numFingers; f++) {
      fp.add(cfg.notes[i].fingerPattern[f] ? 1 : 0);
    }
  }

  // --- Scalaires ---
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
  doc["time_unpower"] = cfg.timeUnpower;
  doc["wifi_ssid"] = cfg.wifiSsid;
  doc["wifi_pass"] = cfg.wifiPassword;
  doc["device"] = cfg.deviceName;

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
