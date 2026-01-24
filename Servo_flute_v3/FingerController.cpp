#include "FingerController.h"

FingerController::FingerController(Adafruit_PWMServoDriver& pwm)
  : _pwm(pwm) {
}

void FingerController::begin() {
  if (DEBUG) {
    Serial.println("DEBUG: FingerController - Initialisation");
    Serial.print("DEBUG:   - Nombre de doigts: ");
    Serial.println(NUMBER_SERVOS_FINGER);
    Serial.print("DEBUG:   - Nombre de notes: ");
    Serial.println(NUMBER_NOTES);
  }
  // Fermer tous les doigts au démarrage
  closeAllFingers();
}

void FingerController::setFingerPattern(const bool pattern[NUMBER_SERVOS_FINGER]) {
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
  // Rechercher la note dans le tableau NOTES
  const NoteDefinition* note = getNoteByMidi(midiNote);

  if (note == nullptr) {
    if (DEBUG) {
      Serial.print("DEBUG: FingerController - Note non trouvée: ");
      Serial.println(midiNote);
    }
    return;
  }

  // Applique le pattern correspondant
  setFingerPattern(note->fingerPattern);

  if (DEBUG) {
    Serial.print("DEBUG: FingerController - Note MIDI: ");
    Serial.println(midiNote);
  }
}

void FingerController::closeAllFingers() {
  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    setServoAngle(i, FINGERS[i].closedAngle);
  }

  if (DEBUG) {
    Serial.println("DEBUG: FingerController - Tous les doigts fermés");
  }
}

void FingerController::openAllFingers() {
  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    uint16_t openAngle = calculateServoAngle(i, true);  // true = ouvert
    setServoAngle(i, openAngle);
  }

  if (DEBUG) {
    Serial.println("DEBUG: FingerController - Tous les doigts ouverts");
  }
}

uint16_t FingerController::calculateServoAngle(int fingerIndex, bool isOpen) {
  uint16_t baseAngle = FINGERS[fingerIndex].closedAngle;

  if (!isOpen) {
    // Fermé : utiliser l'angle de base
    return baseAngle;
  } else {
    // Ouvert : ajouter/soustraire ANGLE_OPEN selon le sens de rotation
    int16_t angle = baseAngle + (ANGLE_OPEN * FINGERS[fingerIndex].direction);

    // Limiter l'angle entre 0 et 180°
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    return (uint16_t)angle;
  }
}

void FingerController::setServoAngle(int fingerIndex, uint16_t angle) {
  // Utiliser directement le canal PCA depuis la structure FINGERS
  int pcaChannel = FINGERS[fingerIndex].pcaChannel;

  uint16_t pwmValue = angleToPWM(angle);
  _pwm.setPWM(pcaChannel, 0, pwmValue);
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
