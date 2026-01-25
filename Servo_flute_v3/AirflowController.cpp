#include "AirflowController.h"

AirflowController::AirflowController(Adafruit_PWMServoDriver& pwm)
  : _pwm(pwm), _solenoidOpen(false), _solenoidOpenTime(0),
    _ccVolume(127), _ccExpression(127), _ccModulation(0) {
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

  // 1. VELOCITY définit l'angle de base dans [minAngle, maxAngle]
  baseAngle = map(velocity, 1, 127, minAngle, maxAngle);

  // ===== APPLICATION DES CONTROL CHANGE =====

  // 2. CC11 (Expression) module DANS la plage [minAngle, baseAngle]
  //    CC11 = 127 → baseAngle (pleine expression selon velocity)
  //    CC11 = 0   → minAngle (expression minimum de la note)
  float expressionFactor = _ccExpression / 127.0;
  float modulatedAngle = minAngle + (baseAngle - minAngle) * expressionFactor;

  // 3. CC7 (Volume) - multiplicateur global
  float finalAngle = modulatedAngle * (_ccVolume / 127.0);

  // 4. CC1 (Modulation) - vibrato
  if (_ccModulation > 0) {
    // Vibrato : oscillation sinusoïdale
    // Fréquence ~6 Hz (période 166ms)
    float vibratoFreq = 6.0;
    float time = millis() / 1000.0;  // Temps en secondes

    // Amplitude du vibrato (max ±8° pour CC1=127)
    float vibratoAmplitude = (_ccModulation / 127.0) * 8.0;

    // Offset sinusoïdal
    float vibratoOffset = sin(2.0 * PI * vibratoFreq * time) * vibratoAmplitude;

    finalAngle += vibratoOffset;
  }

  // 5. Limiter l'angle final dans les bornes valides
  if (finalAngle < SERVO_AIRFLOW_MIN) finalAngle = SERVO_AIRFLOW_MIN;
  if (finalAngle > SERVO_AIRFLOW_MAX) finalAngle = SERVO_AIRFLOW_MAX;

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
    Serial.print(" | CC1: ");
    Serial.print(_ccModulation);
    Serial.print(" | FinalAngle: ");
    Serial.print((uint16_t)finalAngle);
    Serial.println("°");
  }

  setAirflowServoAngle((uint16_t)finalAngle);
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
