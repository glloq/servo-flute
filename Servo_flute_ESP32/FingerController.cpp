#include "FingerController.h"
#include "ConfigStorage.h"

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
  closeAllFingers();
}

void FingerController::setFingerPattern(const bool pattern[NUMBER_SERVOS_FINGER]) {
  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    uint16_t angle = calculateServoAngle(i, pattern[i]);
    setServoAngle(i, angle);
  }

  if (DEBUG) {
    Serial.print("DEBUG: FingerController - Pattern applique: ");
    for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
      Serial.print(pattern[i]);
      Serial.print(" ");
    }
    Serial.println();
  }
}

void FingerController::setFingerPatternForNote(byte midiNote) {
  const NoteDefinition* note = getNoteByMidi(midiNote);

  if (note == nullptr) {
    if (DEBUG) {
      Serial.print("DEBUG: FingerController - Note non trouvee: ");
      Serial.println(midiNote);
    }
    return;
  }

  setFingerPattern(note->fingerPattern);

  if (DEBUG) {
    Serial.print("DEBUG: FingerController - Note MIDI: ");
    Serial.println(midiNote);
  }
}

void FingerController::closeAllFingers() {
  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    setServoAngle(i, cfg.fingerClosedAngle[i]);
  }

  if (DEBUG) {
    Serial.println("DEBUG: FingerController - Tous les doigts fermes");
  }
}

void FingerController::openAllFingers() {
  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    uint16_t openAngle = calculateServoAngle(i, true);
    setServoAngle(i, openAngle);
  }

  if (DEBUG) {
    Serial.println("DEBUG: FingerController - Tous les doigts ouverts");
  }
}

uint16_t FingerController::calculateServoAngle(int fingerIndex, bool isOpen) {
  uint16_t baseAngle = cfg.fingerClosedAngle[fingerIndex];

  if (!isOpen) {
    return baseAngle;
  } else {
    int16_t angle = baseAngle + (cfg.fingerAngleOpen * cfg.fingerDirection[fingerIndex]);

    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    return (uint16_t)angle;
  }
}

void FingerController::setServoAngle(int fingerIndex, uint16_t angle) {
  int pcaChannel = FINGERS[fingerIndex].pcaChannel;
  uint16_t pwmValue = angleToPWM(angle);
  _pwm.setPWM(pcaChannel, 0, pwmValue);
}

uint16_t FingerController::angleToPWM(uint16_t angle) {
  if (angle < SERVO_MIN_ANGLE) angle = SERVO_MIN_ANGLE;
  if (angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;

  uint16_t pulse = map(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE,
                       SERVO_PULSE_MIN, SERVO_PULSE_MAX);

  float pulseDuration = (float)pulse / 1000000.0;
  float pwmValue = pulseDuration * SERVO_FREQUENCY * 4096.0;

  return (uint16_t)pwmValue;
}
