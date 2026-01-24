#include "AirflowController.h"

AirflowController::AirflowController(Adafruit_PWMServoDriver& pwm)
  : _pwm(pwm), _solenoidOpen(false), _solenoidOpenTime(0) {
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
  // Rechercher la note pour obtenir son airflow min spécifique
  const NoteDefinition* note = getNoteByMidi(midiNote);

  uint16_t angle;
  uint16_t minAngle = SERVO_AIRFLOW_MIN;  // Valeur par défaut

  // Si la note définit un airflow min custom, l'utiliser
  if (note != nullptr && note->minAirflow > 0) {
    minAngle = note->minAirflow;
  }

  if (velocity == 0) {
    angle = SERVO_AIRFLOW_OFF;
  } else {
    // Mapping avec le minAngle personnalisé pour cette note
    angle = map(velocity, 1, 127, minAngle, SERVO_AIRFLOW_MAX);
  }

  setAirflowServoAngle(angle);

  if (DEBUG) {
    Serial.print("DEBUG: AirflowController - Note: ");
    if (note != nullptr) {
      Serial.print(note->name);
    } else {
      Serial.print(midiNote);
    }
    Serial.print(" | Vel: ");
    Serial.print(velocity);
    Serial.print(" | Min: ");
    Serial.print(minAngle);
    Serial.print(" | Angle: ");
    Serial.println(angle);
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
