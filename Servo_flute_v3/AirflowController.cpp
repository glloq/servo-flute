#include "AirflowController.h"

// Lookup table pour sin() - 256 entrées pour une période complète [0, 2π]
// Valeurs: -127 à +127 (représente -1.0 à +1.0)
const int8_t SIN_LUT[256] PROGMEM = {
  0, 3, 6, 9, 12, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46,
  49, 51, 54, 57, 60, 63, 65, 68, 71, 73, 76, 78, 81, 83, 85, 88,
  90, 92, 94, 96, 98, 100, 102, 104, 106, 107, 109, 111, 112, 113, 115, 116,
  117, 118, 120, 121, 122, 122, 123, 124, 125, 125, 126, 126, 126, 127, 127, 127,
  127, 127, 127, 127, 126, 126, 126, 125, 125, 124, 123, 122, 122, 121, 120, 118,
  117, 116, 115, 113, 112, 111, 109, 107, 106, 104, 102, 100, 98, 96, 94, 92,
  90, 88, 85, 83, 81, 78, 76, 73, 71, 68, 65, 63, 60, 57, 54, 51,
  49, 46, 43, 40, 37, 34, 31, 28, 25, 22, 19, 16, 12, 9, 6, 3,
  0, -3, -6, -9, -12, -16, -19, -22, -25, -28, -31, -34, -37, -40, -43, -46,
  -49, -51, -54, -57, -60, -63, -65, -68, -71, -73, -76, -78, -81, -83, -85, -88,
  -90, -92, -94, -96, -98, -100, -102, -104, -106, -107, -109, -111, -112, -113, -115, -116,
  -117, -118, -120, -121, -122, -122, -123, -124, -125, -125, -126, -126, -126, -127, -127, -127,
  -127, -127, -127, -127, -126, -126, -126, -125, -125, -124, -123, -122, -122, -121, -120, -118,
  -117, -116, -115, -113, -112, -111, -109, -107, -106, -104, -102, -100, -98, -96, -94, -92,
  -90, -88, -85, -83, -81, -78, -76, -73, -71, -68, -65, -63, -60, -57, -54, -51,
  -49, -46, -43, -40, -37, -34, -31, -28, -25, -22, -19, -16, -12, -9, -6, -3
};

// Fonction helper pour lookup rapide sin()
inline float fastSin(unsigned long timeMs, float frequency) {
  // Calculer phase avec modulo pour éviter overflow
  // Phase en degrés [0, 360) -> index [0, 256)
  unsigned long period = (unsigned long)(1000.0 / frequency);  // Période en ms
  unsigned long phase = timeMs % period;  // Position dans période
  uint8_t index = (uint8_t)((phase * 256UL) / period);  // Index LUT

  return pgm_read_byte(&SIN_LUT[index]) / 127.0;  // Retour -1.0 à +1.0
}

AirflowController::AirflowController(Adafruit_PWMServoDriver& pwm)
  : _pwm(pwm), _solenoidOpen(false), _solenoidOpenTime(0),
    _ccVolume(CC_VOLUME_DEFAULT), _ccExpression(CC_EXPRESSION_DEFAULT), _ccModulation(CC_MODULATION_DEFAULT),
    _pitchBendAdjustment(0),
    _baseAngleWithoutVibrato(SERVO_AIRFLOW_OFF), _vibratoActive(false),
    _currentMinAngle(SERVO_AIRFLOW_MIN), _currentMaxAngle(SERVO_AIRFLOW_MAX) {
}

void AirflowController::begin() {
  // Configurer le pin du solénoïde en sortie
  pinMode(SOLENOID_PIN, OUTPUT);

  // Fermer le solénoïde au démarrage
  closeSolenoid();

  // Positionner le servo de débit en position repos
  setAirflowToRest();

  if (DEBUG) {
    Serial.println("DEBUG: AirflowController - Initialisation");
    #if SOLENOID_USE_PWM
    Serial.println("DEBUG: AirflowController - Mode PWM activé");
    Serial.print("DEBUG:   - PWM activation: ");
    Serial.println(SOLENOID_PWM_ACTIVATION);
    Serial.print("DEBUG:   - PWM maintien: ");
    Serial.println(SOLENOID_PWM_HOLDING);
    #else
    Serial.println("DEBUG: AirflowController - Mode GPIO simple");
    #endif
  }
}

void AirflowController::setAirflowVelocity(byte velocity) {
  // Mapper la vélocité MIDI (1-127) vers l'angle du servo
  uint16_t angle;

  if (velocity == 0) {
    angle = SERVO_AIRFLOW_OFF;  // Pas de note
  } else {
    // Mapping linéaire de velocity (1-127) vers angle (min-max)
    angle = map(velocity, 1, 127, SERVO_AIRFLOW_MIN, SERVO_AIRFLOW_MAX);
  }

  setAirflowServoAngle(angle);

  if (DEBUG) {
    Serial.print("DEBUG: AirflowController - Vélocité: ");
    Serial.print(velocity);
    Serial.print(" -> Angle: ");
    Serial.println(angle);
  }
}

void AirflowController::setAirflowForNote(byte midiNote, byte velocity) {
  // Rechercher la note pour obtenir ses pourcentages airflow
  const NoteDefinition* note = getNoteByMidi(midiNote);

  uint16_t minAngle, maxAngle;
  uint16_t baseAngle;

  if (velocity == 0) {
    setAirflowServoAngle(SERVO_AIRFLOW_OFF);
    return;
  }

  // Calculer les angles min/max de la note
  if (note != nullptr) {
    minAngle = SERVO_AIRFLOW_MIN + ((SERVO_AIRFLOW_MAX - SERVO_AIRFLOW_MIN) * note->airflowMinPercent / 100);
    maxAngle = SERVO_AIRFLOW_MIN + ((SERVO_AIRFLOW_MAX - SERVO_AIRFLOW_MIN) * note->airflowMaxPercent / 100);
  } else {
    // Note non trouvée, utiliser plage par défaut
    minAngle = SERVO_AIRFLOW_MIN;
    maxAngle = SERVO_AIRFLOW_MAX;
  }

  // Stocker les bornes pour limiter le vibrato ultérieurement
  _currentMinAngle = minAngle;
  _currentMaxAngle = maxAngle;

  // 1. VELOCITY définit l'angle de base dans [minAngle, maxAngle]
  baseAngle = map(velocity, 1, 127, minAngle, maxAngle);

  // ===== APPLICATION DES CONTROL CHANGE =====

  // 2. CC11 (Expression) module DANS la plage [minAngle, baseAngle]
  //    CC11 = 127 → baseAngle (pleine expression selon velocity)
  //    CC11 = 0   → minAngle (expression minimum de la note)
  float expressionFactor = _ccExpression / 127.0;
  float modulatedAngle = minAngle + (baseAngle - minAngle) * expressionFactor;

  // 3. CC7 (Volume) module DANS la plage [minAngle, modulatedAngle]
  //    CC7 = 127 → modulatedAngle (plein volume)
  //    CC7 = 0   → minAngle (silence minimum de la note)
  //    Garantit que l'angle ne descend JAMAIS sous minAngle
  float volumeFactor = _ccVolume / 127.0;
  float finalAngleWithoutVibrato = minAngle + (modulatedAngle - minAngle) * volumeFactor;

  // 4. Pitch Bend : ajustement fin de l'airflow (±PITCH_BEND_AIRFLOW_PERCENT%)
  //    Appliqué APRÈS tous les CC pour modulation fine
  if (_pitchBendAdjustment != 0) {
    float pitchBendOffset = (finalAngleWithoutVibrato - minAngle) * (_pitchBendAdjustment / 100.0);
    finalAngleWithoutVibrato += pitchBendOffset;
  }

  // 5. Limiter dans les bornes valides
  if (finalAngleWithoutVibrato < SERVO_AIRFLOW_MIN) finalAngleWithoutVibrato = SERVO_AIRFLOW_MIN;
  if (finalAngleWithoutVibrato > SERVO_AIRFLOW_MAX) finalAngleWithoutVibrato = SERVO_AIRFLOW_MAX;

  // Stocker l'angle de base (sans vibrato) pour update continu
  _baseAngleWithoutVibrato = (uint16_t)(finalAngleWithoutVibrato + 0.5);  // Arrondi

  // Activer vibrato si CC1 > 0
  _vibratoActive = (_ccModulation > 0);

  if (DEBUG) {
    Serial.print("DEBUG: AirflowController - Note MIDI: ");
    Serial.print(midiNote);
    Serial.print(" | Vel: ");
    Serial.print(velocity);
    Serial.print(" | Range: ");
    Serial.print(minAngle);
    Serial.print("°-");
    Serial.print(maxAngle);
    Serial.print("° | BaseAngle: ");
    Serial.print(baseAngle);
    Serial.print("° | CC11: ");
    Serial.print(_ccExpression);
    Serial.print(" → ");
    Serial.print((uint16_t)modulatedAngle);
    Serial.print("° | CC7: ");
    Serial.print(_ccVolume);
    Serial.print(" | Base(no vib): ");
    Serial.print(_baseAngleWithoutVibrato);
    Serial.print("° | CC1: ");
    Serial.print(_ccModulation);
    if (_vibratoActive) {
      Serial.print(" (vibrato ON)");
    }
    Serial.println();
  }

  // Appliquer immédiatement (update() ajoutera vibrato si nécessaire)
  if (_vibratoActive) {
    // update() gérera le vibrato en continu
    update();  // Premier calcul immédiat
  } else {
    setAirflowServoAngle(_baseAngleWithoutVibrato);
  }
}

void AirflowController::openSolenoid() {
  #if SOLENOID_USE_PWM
    // Mode PWM : démarrer à pleine puissance pour ouverture rapide
    setSolenoidPWM(SOLENOID_PWM_ACTIVATION);
    _solenoidOpenTime = millis();  // Sauvegarder timestamp pour réduction ultérieure
  #else
    // Mode GPIO simple
    if (SOLENOID_ACTIVE_HIGH) {
      digitalWrite(SOLENOID_PIN, HIGH);
    } else {
      digitalWrite(SOLENOID_PIN, LOW);
    }
  #endif

  _solenoidOpen = true;

  if (DEBUG) {
    #if SOLENOID_USE_PWM
    Serial.print("DEBUG: AirflowController - Solénoïde OUVERT (PWM=");
    Serial.print(SOLENOID_PWM_ACTIVATION);
    Serial.println(")");
    #else
    Serial.println("DEBUG: AirflowController - Solénoïde OUVERT");
    #endif
  }
}

void AirflowController::closeSolenoid() {
  #if SOLENOID_USE_PWM
    // Mode PWM : mettre à 0
    setSolenoidPWM(0);
  #else
    // Mode GPIO simple
    if (SOLENOID_ACTIVE_HIGH) {
      digitalWrite(SOLENOID_PIN, LOW);
    } else {
      digitalWrite(SOLENOID_PIN, HIGH);
    }
  #endif

  _solenoidOpen = false;
  _solenoidOpenTime = 0;

  if (DEBUG) {
    Serial.println("DEBUG: AirflowController - Solénoïde FERMÉ");
  }
}

bool AirflowController::isSolenoidOpen() const {
  return _solenoidOpen;
}

void AirflowController::setAirflowToRest() {
  setAirflowServoAngle(SERVO_AIRFLOW_OFF);

  if (DEBUG) {
    Serial.println("DEBUG: AirflowController - Servo en position repos");
  }
}

void AirflowController::update() {
  #if SOLENOID_USE_PWM
  // Gérer la réduction PWM après SOLENOID_ACTIVATION_TIME_MS
  if (_solenoidOpen && _solenoidOpenTime > 0) {
    unsigned long elapsed = millis() - _solenoidOpenTime;

    if (elapsed >= SOLENOID_ACTIVATION_TIME_MS) {
      // Réduire le PWM pour maintien (économie énergie/chaleur)
      setSolenoidPWM(SOLENOID_PWM_HOLDING);
      _solenoidOpenTime = 0;  // Reset pour ne faire qu'une fois

      if (DEBUG) {
        Serial.print("DEBUG: AirflowController - PWM réduit à ");
        Serial.print(SOLENOID_PWM_HOLDING);
        Serial.println(" (maintien)");
      }
    }
  }
  #endif

  // Appliquer vibrato si actif
  if (_vibratoActive && _ccModulation > 0 && _solenoidOpen) {
    // Fréquence vibrato: ~6 Hz (standard musical)
    const float VIBRATO_FREQUENCY = VIBRATO_FREQUENCY_HZ;

    // Amplitude max: ±8° (modulation 127 = ±8°, modulation 64 = ±4°, etc.)
    float vibratoAmplitude = (_ccModulation / 127.0) * VIBRATO_MAX_AMPLITUDE_DEG;

    // Calculer offset vibrato avec sin() optimisé
    float vibratoOffset = fastSin(millis(), VIBRATO_FREQUENCY) * vibratoAmplitude;

    // Appliquer vibrato à l'angle de base
    int16_t finalAngle = _baseAngleWithoutVibrato + (int16_t)(vibratoOffset + 0.5);

    // Limiter dans les bornes de la note en cours (pas les bornes servo globales)
    if (finalAngle < (int16_t)_currentMinAngle) finalAngle = _currentMinAngle;
    if (finalAngle > (int16_t)_currentMaxAngle) finalAngle = _currentMaxAngle;

    // Mettre à jour position servo
    setAirflowServoAngle((uint16_t)finalAngle);
  }
}

void AirflowController::setAirflowServoAngle(uint16_t angle) {
  uint16_t pwmValue = angleToPWM(angle);
  _pwm.setPWM(NUM_SERVO_AIRFLOW, 0, pwmValue);
}

uint16_t AirflowController::angleToPWM(uint16_t angle) {
  // Limiter l'angle entre min et max
  if (angle < SERVO_MIN_ANGLE) angle = SERVO_MIN_ANGLE;
  if (angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;

  // Convertir angle en largeur d'impulsion (µs)
  uint16_t pulse = map(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE,
                       SERVO_PULSE_MIN, SERVO_PULSE_MAX);

  // Convertir impulsion en valeur PWM pour PCA9685
  float pulseDuration = (float)pulse / 1000000.0;
  float pwmValue = pulseDuration * SERVO_FREQUENCY * 4096.0;

  return (uint16_t)pwmValue;
}

void AirflowController::setSolenoidPWM(uint8_t pwmValue) {
  #if SOLENOID_USE_PWM
    if (SOLENOID_ACTIVE_HIGH) {
      analogWrite(SOLENOID_PIN, pwmValue);
    } else {
      analogWrite(SOLENOID_PIN, 255 - pwmValue);  // Inverser si active low
    }
  #endif
}

void AirflowController::setCCValues(byte ccVolume, byte ccExpression, byte ccModulation) {
  _ccVolume = ccVolume;
  _ccExpression = ccExpression;
  _ccModulation = ccModulation;
}

void AirflowController::setPitchBendAdjustment(int8_t adjustment) {
  _pitchBendAdjustment = adjustment;

  // Limiter à ±PITCH_BEND_AIRFLOW_PERCENT
  if (_pitchBendAdjustment > PITCH_BEND_AIRFLOW_PERCENT) {
    _pitchBendAdjustment = PITCH_BEND_AIRFLOW_PERCENT;
  }
  if (_pitchBendAdjustment < -PITCH_BEND_AIRFLOW_PERCENT) {
    _pitchBendAdjustment = -PITCH_BEND_AIRFLOW_PERCENT;
  }
}
