#include "FingerController.h"

FingerController::FingerController(Adafruit_PWMServoDriver& pwm)
  : _pwm(pwm) {
}

void FingerController::begin() {
  if (DEBUG) {
    Serial.println("DEBUG: FingerController - Initialisation");
  }
  // Fermer tous les doigts au démarrage
  closeAllFingers();
}

void FingerController::setFingerPattern(const int pattern[NUMBER_SERVOS_FINGER]) {
  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    uint16_t angle = calculateServoAngle(i, pattern[i]);
    setServoAngle(i, angle);
  }

  if (DEBUG) {
    Serial.print("DEBUG: FingerController - Pattern appliqué: ");
    for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
      Serial.print(pattern[i]);
      Serial.print(" ");
    }
    Serial.println();
  }
}

void FingerController::setFingerPatternForNote(byte midiNote) {
  // Vérifie si la note est dans la plage jouable
  if (midiNote < FIRST_MIDI_NOTE || midiNote >= (FIRST_MIDI_NOTE + NUMBER_NOTES)) {
    if (DEBUG) {
      Serial.print("DEBUG: FingerController - Note hors plage: ");
      Serial.println(midiNote);
    }
    return;
  }

  // Calcule l'index dans le tableau de doigtés
  int noteIndex = midiNote - FIRST_MIDI_NOTE;

  // Applique le pattern correspondant
  setFingerPattern(finger_position[noteIndex]);
}

void FingerController::closeAllFingers() {
  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    setServoAngle(i, closedAngles[i]);
  }

  if (DEBUG) {
    Serial.println("DEBUG: FingerController - Tous les doigts fermés");
  }
}

void FingerController::openAllFingers() {
  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    uint16_t openAngle = calculateServoAngle(i, 1);  // 1 = ouvert
    setServoAngle(i, openAngle);
  }

  if (DEBUG) {
    Serial.println("DEBUG: FingerController - Tous les doigts ouverts");
  }
}

uint16_t FingerController::calculateServoAngle(int servoIndex, int state) {
  uint16_t baseAngle = closedAngles[servoIndex];

  if (state == 0) {
    // Fermé : utiliser l'angle de base
    return baseAngle;
  } else {
    // Ouvert : ajouter/soustraire ANGLE_OPEN selon le sens de rotation
    int16_t angle = baseAngle + (ANGLE_OPEN * sensRotation[servoIndex]);

    // Limiter l'angle entre 0 et 180°
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    return (uint16_t)angle;
  }
}

void FingerController::setServoAngle(int servoIndex, uint16_t angle) {
  uint16_t pwmValue = angleToPWM(angle);
  _pwm.setPWM(servoIndex, 0, pwmValue);
}

uint16_t FingerController::angleToPWM(uint16_t angle) {
  // Limiter l'angle entre min et max
  if (angle < SERVO_MIN_ANGLE) angle = SERVO_MIN_ANGLE;
  if (angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;

  // Convertir angle en largeur d'impulsion (µs)
  uint16_t pulse = map(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE,
                       SERVO_PULSE_MIN, SERVO_PULSE_MAX);

  // Convertir impulsion en valeur PWM pour PCA9685
  // Formule: (pulse_µs / 1000000) * fréquence * 4096
  float pulseDuration = (float)pulse / 1000000.0;
  float pwmValue = pulseDuration * SERVO_FREQUENCY * 4096.0;

  return (uint16_t)pwmValue;
}
