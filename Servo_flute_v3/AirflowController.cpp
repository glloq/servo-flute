#include "AirflowController.h"

AirflowController::AirflowController(Adafruit_PWMServoDriver& pwm)
  : _pwm(pwm), _solenoidOpen(false) {
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

void AirflowController::openSolenoid() {
  if (SOLENOID_ACTIVE_HIGH) {
    digitalWrite(SOLENOID_PIN, HIGH);
  } else {
    digitalWrite(SOLENOID_PIN, LOW);
  }

  _solenoidOpen = true;

  if (DEBUG) {
    Serial.println("DEBUG: AirflowController - Solénoïde OUVERT");
  }
}

void AirflowController::closeSolenoid() {
  if (SOLENOID_ACTIVE_HIGH) {
    digitalWrite(SOLENOID_PIN, LOW);
  } else {
    digitalWrite(SOLENOID_PIN, HIGH);
  }

  _solenoidOpen = false;

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
