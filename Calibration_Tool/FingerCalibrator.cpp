/***********************************************************************************************
 * FINGER CALIBRATOR - IMPLEMENTATION
 ***********************************************************************************************/
#include "FingerCalibrator.h"

FingerCalibrator::FingerCalibrator(Adafruit_PWMServoDriver& pwm)
  : _pwm(pwm), _currentAngle(90), _currentDirection(1), _step(1) {
}

void FingerCalibrator::calibrateFinger(int fingerIndex, FingerConfig& output) {
  _currentFingerIndex = fingerIndex;
  byte pcaChannel = FINGERS_TEMPLATE[fingerIndex].pcaChannel;

  Serial.println(F("\n========================================"));
  Serial.print(F("  CALIBRATION SERVO DOIGT "));
  Serial.print(fingerIndex + 1);
  Serial.print(F("/"));
  Serial.println(NUMBER_SERVOS_FINGER);
  Serial.println(F("========================================"));
  Serial.print(F("Canal PCA9685: "));
  Serial.println(pcaChannel);
  Serial.print(F("Trou: "));
  Serial.print(fingerIndex + 1);

  if (fingerIndex == 0) Serial.println(F(" (haut, main gauche - index)"));
  else if (fingerIndex == 1) Serial.println(F(" (main gauche - majeur)"));
  else if (fingerIndex == 2) Serial.println(F(" (main gauche - annulaire)"));
  else if (fingerIndex == 3) Serial.println(F(" (main droite - index)"));
  else if (fingerIndex == 4) Serial.println(F(" (main droite - majeur)"));
  else if (fingerIndex == 5) Serial.println(F(" (bas, main droite - annulaire)"));

  Serial.println();

  // Étape 1 : Calibrer l'angle fermé
  calibrateClosedAngle(pcaChannel);
  uint16_t closedAngle = _currentAngle;

  // Étape 2 : Déterminer le sens de rotation
  calibrateDirection(pcaChannel, closedAngle);
  int8_t direction = _currentDirection;

  // Étape 3 : Vérification finale
  bool confirmed = verifyConfiguration(pcaChannel, closedAngle, direction);

  if (confirmed) {
    output.pcaChannel = pcaChannel;
    output.closedAngle = closedAngle;
    output.direction = direction;

    Serial.println();
    Serial.print(F("✓ Doigt "));
    Serial.print(fingerIndex + 1);
    Serial.print(F("/"));
    Serial.print(NUMBER_SERVOS_FINGER);
    Serial.println(F(" calibré!"));
  } else {
    Serial.println(F("❌ Calibration annulée"));
  }
}

void FingerCalibrator::calibrateClosedAngle(byte pcaChannel) {
  _currentAngle = 90;  // Angle de départ

  Serial.println(F("ÉTAPE 1/3 - Trouver l'angle FERMÉ"));
  Serial.println(F("----------------------------------"));
  Serial.println(F("Instructions:"));
  Serial.println(F("  - Le trou doit être complètement BOUCHÉ"));
  Serial.println(F("  - Ajustez jusqu'à fermeture hermétique"));
  Serial.println(F("  - Ne pas forcer le servo"));
  Serial.println();
  Serial.println(F("Commandes:"));
  Serial.println(F("  +         : Augmenter angle (+1°)"));
  Serial.println(F("  -         : Diminuer angle (-1°)"));
  Serial.println(F("  > ou ]    : Augmenter rapide (+5°)"));
  Serial.println(F("  < ou [    : Diminuer rapide (-5°)"));
  Serial.println(F("  t         : Tester position actuelle"));
  Serial.println(F("  s         : Sauvegarder et continuer"));
  Serial.println();

  setServoAngle(pcaChannel, _currentAngle);
  Serial.print(F("Position actuelle: "));
  Serial.print(_currentAngle);
  Serial.println(F("°"));
  Serial.println();

  bool done = false;
  while (!done) {
    if (Serial.available() > 0) {
      char cmd = Serial.read();

      switch (cmd) {
        case '+':
          adjustAngle(1);
          setServoAngle(pcaChannel, _currentAngle);
          Serial.print(F("Position: "));
          Serial.print(_currentAngle);
          Serial.println(F("° [APPLIQUÉ]"));
          break;

        case '-':
          adjustAngle(-1);
          setServoAngle(pcaChannel, _currentAngle);
          Serial.print(F("Position: "));
          Serial.print(_currentAngle);
          Serial.println(F("° [APPLIQUÉ]"));
          break;

        case '>':
        case ']':
          adjustAngle(5);
          setServoAngle(pcaChannel, _currentAngle);
          Serial.print(F("Position: "));
          Serial.print(_currentAngle);
          Serial.println(F("° [APPLIQUÉ]"));
          break;

        case '<':
        case '[':
          adjustAngle(-5);
          setServoAngle(pcaChannel, _currentAngle);
          Serial.print(F("Position: "));
          Serial.print(_currentAngle);
          Serial.println(F("° [APPLIQUÉ]"));
          break;

        case 't':
        case 'T':
          testCurrentPosition(pcaChannel);
          break;

        case 's':
        case 'S':
          Serial.println();
          Serial.print(F("Angle fermé confirmé: "));
          Serial.print(_currentAngle);
          Serial.println(F("°"));
          done = true;
          break;
      }
    }
  }
}

void FingerCalibrator::calibrateDirection(byte pcaChannel, uint16_t closedAngle) {
  Serial.println();
  Serial.println(F("ÉTAPE 2/3 - Déterminer le SENS de rotation"));
  Serial.println(F("-------------------------------------------"));
  Serial.println(F("Instruction:"));
  Serial.println(F("  - Le servo va s'ouvrir de 30°"));
  Serial.println(F("  - Vérifiez que le trou S'OUVRE (ne se ferme pas plus)"));
  Serial.println();

  // Test sens horaire (+1)
  Serial.println(F("Test avec sens HORAIRE (+1):"));
  Serial.print(F("  Fermé: "));
  Serial.print(closedAngle);
  Serial.print(F("° → Ouvert: "));
  Serial.print(closedAngle + ANGLE_OPEN);
  Serial.println(F("°"));
  Serial.println();

  setServoAngle(pcaChannel, closedAngle);
  delay(500);
  setServoAngle(pcaChannel, closedAngle + ANGLE_OPEN);
  delay(1000);
  setServoAngle(pcaChannel, closedAngle);
  delay(500);

  Serial.print(F("Le trou s'ouvre-t-il correctement? (o/n): "));
  bool horaireFonctionne = waitForConfirmation();

  if (horaireFunctionne) {
    _currentDirection = 1;
    Serial.println(F("Sens confirmé: 1 (horaire)"));
    return;
  }

  Serial.println();

  // Test sens anti-horaire (-1)
  Serial.println(F("Test avec sens ANTI-HORAIRE (-1):"));
  Serial.print(F("  Fermé: "));
  Serial.print(closedAngle);
  Serial.print(F("° → Ouvert: "));
  Serial.print(closedAngle - ANGLE_OPEN);
  Serial.println(F("°"));
  Serial.println();

  setServoAngle(pcaChannel, closedAngle);
  delay(500);
  setServoAngle(pcaChannel, closedAngle - ANGLE_OPEN);
  delay(1000);
  setServoAngle(pcaChannel, closedAngle);
  delay(500);

  Serial.print(F("Le trou s'ouvre-t-il correctement? (o/n): "));
  bool antihoraireFonctionne = waitForConfirmation();

  if (antihoraireFonctionne) {
    _currentDirection = -1;
    Serial.println(F("Sens confirmé: -1 (anti-horaire)"));
  } else {
    Serial.println(F("⚠ ATTENTION: Aucun sens ne semble fonctionner!"));
    Serial.println(F("Utilisation par défaut: -1 (anti-horaire)"));
    _currentDirection = -1;
  }
}

bool FingerCalibrator::verifyConfiguration(byte pcaChannel, uint16_t closedAngle, int8_t direction) {
  Serial.println();
  Serial.println(F("ÉTAPE 3/3 - Vérification finale"));
  Serial.println(F("--------------------------------"));
  Serial.print(F("Configuration doigt "));
  Serial.print(_currentFingerIndex + 1);
  Serial.println(F(":"));
  Serial.print(F("  Canal PCA: "));
  Serial.println(pcaChannel);
  Serial.print(F("  Angle fermé: "));
  Serial.print(closedAngle);
  Serial.println(F("°"));
  Serial.print(F("  Sens: "));
  Serial.print(direction);
  Serial.print(F(" ("));
  Serial.print(direction == 1 ? F("horaire") : F("anti-horaire"));
  Serial.println(F(")"));
  Serial.print(F("  Angle ouvert: "));
  Serial.print(closedAngle + (ANGLE_OPEN * direction));
  Serial.print(F("° ("));
  Serial.print(closedAngle);
  Serial.print(F("° + "));
  Serial.print(ANGLE_OPEN);
  Serial.print(F("° × "));
  Serial.print(direction);
  Serial.println(F(")"));
  Serial.println();

  Serial.print(F("Tester? (o/n): "));
  if (waitForConfirmation()) {
    testOpenClose(pcaChannel, closedAngle, direction);
  }

  Serial.println();
  Serial.print(F("Confirmer et passer au suivant? (o/n): "));
  return waitForConfirmation();
}

void FingerCalibrator::setServoAngle(byte pcaChannel, uint16_t angle) {
  uint16_t pwm = angleToPWM(angle);
  _pwm.setPWM(pcaChannel, 0, pwm);
}

uint16_t FingerCalibrator::angleToPWM(uint16_t angle) {
  return map(angle, 0, 180, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
}

void FingerCalibrator::adjustAngle(int delta) {
  int newAngle = (int)_currentAngle + delta;
  if (newAngle < 0) newAngle = 0;
  if (newAngle > 180) newAngle = 180;
  _currentAngle = (uint16_t)newAngle;
}

void FingerCalibrator::testCurrentPosition(byte pcaChannel) {
  Serial.print(F("[TEST] Position "));
  Serial.print(_currentAngle);
  Serial.println(F("°"));
  setServoAngle(pcaChannel, _currentAngle);
}

void FingerCalibrator::testOpenClose(byte pcaChannel, uint16_t closedAngle, int8_t direction) {
  uint16_t openAngle = closedAngle + (ANGLE_OPEN * direction);

  Serial.println(F("[TEST] Oscillation ouvert/fermé (3 cycles)"));
  for (int i = 0; i < 3; i++) {
    Serial.print(F("  Cycle "));
    Serial.print(i + 1);
    Serial.print(F("/3: "));
    Serial.print(closedAngle);
    Serial.print(F("° → "));
    Serial.print(openAngle);
    Serial.println(F("°"));

    setServoAngle(pcaChannel, closedAngle);
    delay(500);
    setServoAngle(pcaChannel, openAngle);
    delay(500);
  }

  // Retour à fermé
  setServoAngle(pcaChannel, closedAngle);
  Serial.println(F("[TEST] Terminé"));
}

bool FingerCalibrator::waitForConfirmation() {
  while (true) {
    while (Serial.available() == 0) {
      // Attendre
    }

    char c = Serial.read();
    if (c == 'o' || c == 'O' || c == 'y' || c == 'Y') {
      Serial.println(F("o"));
      return true;
    } else if (c == 'n' || c == 'N') {
      Serial.println(F("n"));
      return false;
    }
  }
}
