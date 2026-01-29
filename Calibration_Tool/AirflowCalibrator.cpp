/***********************************************************************************************
 * AIRFLOW CALIBRATOR - IMPLEMENTATION
 ***********************************************************************************************/
#include "AirflowCalibrator.h"

AirflowCalibrator::AirflowCalibrator(Adafruit_PWMServoDriver& pwm)
  : _pwm(pwm), _currentPercent(0), _step(1) {
}

void AirflowCalibrator::calibrateNote(int noteIndex,
                                      const FingerConfig calibratedFingers[],
                                      NoteDefinition& output) {
  _currentNoteIndex = noteIndex;

  // Copier le template de la note
  output = NOTES_TEMPLATE[noteIndex];

  Serial.println(F("\n========================================"));
  Serial.print(F("  CALIBRATION AIRFLOW (Note "));
  Serial.print(noteIndex + 1);
  Serial.print(F("/"));
  Serial.print(NUMBER_NOTES);
  Serial.println(F(")"));
  Serial.println(F("========================================"));
  Serial.print(F("Note: "));
  Serial.print(NOTE_NAMES[noteIndex]);
  Serial.print(F(" (MIDI "));
  Serial.print(output.midiNote);
  Serial.println(F(")"));

  Serial.print(F("Doigté: "));
  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    Serial.print(output.fingerPattern[i]);
  }
  Serial.print(F(" ("));

  int closedCount = 0;
  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    if (output.fingerPattern[i] == 0) closedCount++;
  }
  Serial.print(closedCount);
  Serial.print(F(" fermé"));
  if (closedCount > 1) Serial.print(F("s"));
  Serial.println(F(")"));
  Serial.println();

  // Préparer le système
  Serial.println(F("PRÉPARATION"));
  Serial.println(F("-----------"));
  Serial.println(F("1. Application doigtés"));
  applyFingering(output.fingerPattern, calibratedFingers);
  delay(500);

  Serial.println(F("2. Ouverture solénoïde"));
  openSolenoid();
  delay(200);

  Serial.println(F("3. Prêt pour ajustement airflow"));
  Serial.println();

  // Étape 1 : Calibrer airflowMinPercent
  calibrateMinPercent();
  output.airflowMinPercent = _currentPercent;

  // Étape 2 : Calibrer airflowMaxPercent
  calibrateMaxPercent(output.airflowMinPercent);
  output.airflowMaxPercent = _currentPercent;

  // Vérification finale
  bool confirmed = verifyConfiguration(output);

  // Fermer le solénoïde
  closeSolenoid();

  // Remettre airflow au repos
  setAirflowPercent(0);
  setServoAngle(NUM_SERVO_AIRFLOW, SERVO_AIRFLOW_OFF);

  if (confirmed) {
    Serial.println();
    Serial.print(F("✓ Note "));
    Serial.print(noteIndex + 1);
    Serial.print(F("/"));
    Serial.print(NUMBER_NOTES);
    Serial.println(F(" calibrée!"));
  } else {
    Serial.println(F("❌ Calibration annulée"));
  }
}

void AirflowCalibrator::applyFingering(const bool fingerPattern[],
                                       const FingerConfig calibratedFingers[]) {
  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    uint16_t angle;

    if (fingerPattern[i] == 0) {
      // Fermé
      angle = calibratedFingers[i].closedAngle;
    } else {
      // Ouvert
      angle = calibratedFingers[i].closedAngle +
              (ANGLE_OPEN * calibratedFingers[i].direction);
    }

    setServoAngle(calibratedFingers[i].pcaChannel, angle);
  }

  Serial.println(F("  [Servos positionnés selon doigté]"));
}

void AirflowCalibrator::calibrateMinPercent() {
  _currentPercent = 0;

  Serial.println(F("ÉTAPE 1/2 - Trouver airflowMinPercent"));
  Serial.println(F("--------------------------------------"));
  Serial.println(F("Instructions:"));
  Serial.println(F("  - Cherchez le % MINIMUM pour que la note sonne"));
  Serial.println(F("  - Note doit être stable et juste"));
  Serial.println(F("  - Si trop faible: note ne sonne pas"));
  Serial.println();
  Serial.println(F("Commandes:"));
  Serial.println(F("  +         : Augmenter (+1%)"));
  Serial.println(F("  -         : Diminuer (-1%)"));
  Serial.println(F("  > ou ]    : Augmenter rapide (+5%)"));
  Serial.println(F("  < ou [    : Diminuer rapide (-5%)"));
  Serial.println(F("  t         : Tester note (2 secondes)"));
  Serial.println(F("  s         : Sauvegarder"));
  Serial.println();

  setAirflowPercent(_currentPercent);
  uint16_t angle = percentToAngle(_currentPercent);
  Serial.print(F("Angle actuel: "));
  Serial.print(angle);
  Serial.print(F("° - Pourcentage: "));
  Serial.print(_currentPercent);
  Serial.println(F("%"));
  Serial.println();

  bool done = false;
  while (!done) {
    if (Serial.available() > 0) {
      char cmd = Serial.read();

      switch (cmd) {
        case '+':
          adjustPercent(1);
          setAirflowPercent(_currentPercent);
          angle = percentToAngle(_currentPercent);
          Serial.print(F("Angle: "));
          Serial.print(angle);
          Serial.print(F("° - Pourcentage: "));
          Serial.print(_currentPercent);
          Serial.println(F("% [APPLIQUÉ]"));
          break;

        case '-':
          adjustPercent(-1);
          setAirflowPercent(_currentPercent);
          angle = percentToAngle(_currentPercent);
          Serial.print(F("Angle: "));
          Serial.print(angle);
          Serial.print(F("° - Pourcentage: "));
          Serial.print(_currentPercent);
          Serial.println(F("% [APPLIQUÉ]"));
          break;

        case '>':
        case ']':
          adjustPercent(5);
          setAirflowPercent(_currentPercent);
          angle = percentToAngle(_currentPercent);
          Serial.print(F("Angle: "));
          Serial.print(angle);
          Serial.print(F("° - Pourcentage: "));
          Serial.print(_currentPercent);
          Serial.println(F("% [APPLIQUÉ]"));
          break;

        case '<':
        case '[':
          adjustPercent(-5);
          setAirflowPercent(_currentPercent);
          angle = percentToAngle(_currentPercent);
          Serial.print(F("Angle: "));
          Serial.print(angle);
          Serial.print(F("° - Pourcentage: "));
          Serial.print(_currentPercent);
          Serial.println(F("% [APPLIQUÉ]"));
          break;

        case 't':
        case 'T':
          testNote(2000);
          break;

        case 's':
        case 'S':
          Serial.println();
          Serial.print(F("airflowMinPercent confirmé: "));
          Serial.print(_currentPercent);
          Serial.println(F("%"));
          done = true;
          break;
      }
    }
  }
}

void AirflowCalibrator::calibrateMaxPercent(byte minPercent) {
  // Démarrer à 50% pour accélérer
  _currentPercent = 50;
  if (_currentPercent <= minPercent) {
    _currentPercent = minPercent + 10;
  }

  Serial.println();
  Serial.println(F("ÉTAPE 2/2 - Trouver airflowMaxPercent"));
  Serial.println(F("--------------------------------------"));
  Serial.println(F("Instructions:"));
  Serial.println(F("  - Cherchez le % MAXIMUM avant sur-soufflage"));
  Serial.println(F("  - Note doit rester stable (pas de sifflement)"));
  Serial.println(F("  - Si trop fort: note monte d'octave ou siffle"));
  Serial.println();
  Serial.print(F("Position actuelle: "));
  Serial.print(minPercent);
  Serial.println(F("% (minimum)"));
  Serial.print(F("Saut automatique à "));
  Serial.print(_currentPercent);
  Serial.println(F("% pour accélérer"));
  Serial.println();

  setAirflowPercent(_currentPercent);
  uint16_t angle = percentToAngle(_currentPercent);
  Serial.print(F("Angle: "));
  Serial.print(angle);
  Serial.print(F("° - Pourcentage: "));
  Serial.print(_currentPercent);
  Serial.println(F("%"));
  Serial.println();
  Serial.println(F("Commandes: mêmes que étape 1"));
  Serial.println();

  bool done = false;
  while (!done) {
    if (Serial.available() > 0) {
      char cmd = Serial.read();

      switch (cmd) {
        case '+':
          adjustPercent(1);
          if (_currentPercent < minPercent) _currentPercent = minPercent;
          setAirflowPercent(_currentPercent);
          angle = percentToAngle(_currentPercent);
          Serial.print(F("Angle: "));
          Serial.print(angle);
          Serial.print(F("° - Pourcentage: "));
          Serial.print(_currentPercent);
          Serial.println(F("% [APPLIQUÉ]"));
          break;

        case '-':
          adjustPercent(-1);
          if (_currentPercent < minPercent) _currentPercent = minPercent;
          setAirflowPercent(_currentPercent);
          angle = percentToAngle(_currentPercent);
          Serial.print(F("Angle: "));
          Serial.print(angle);
          Serial.print(F("° - Pourcentage: "));
          Serial.print(_currentPercent);
          Serial.println(F("% [APPLIQUÉ]"));
          break;

        case '>':
        case ']':
          adjustPercent(5);
          if (_currentPercent < minPercent) _currentPercent = minPercent;
          setAirflowPercent(_currentPercent);
          angle = percentToAngle(_currentPercent);
          Serial.print(F("Angle: "));
          Serial.print(angle);
          Serial.print(F("° - Pourcentage: "));
          Serial.print(_currentPercent);
          Serial.println(F("% [APPLIQUÉ]"));
          break;

        case '<':
        case '[':
          adjustPercent(-5);
          if (_currentPercent < minPercent) _currentPercent = minPercent;
          setAirflowPercent(_currentPercent);
          angle = percentToAngle(_currentPercent);
          Serial.print(F("Angle: "));
          Serial.print(angle);
          Serial.print(F("° - Pourcentage: "));
          Serial.print(_currentPercent);
          Serial.println(F("% [APPLIQUÉ]"));
          break;

        case 't':
        case 'T':
          testNote(2000);
          break;

        case 's':
        case 'S':
          Serial.println();
          Serial.print(F("airflowMaxPercent confirmé: "));
          Serial.print(_currentPercent);
          Serial.println(F("%"));
          done = true;
          break;
      }
    }
  }
}

bool AirflowCalibrator::verifyConfiguration(const NoteDefinition& note) {
  Serial.println();
  Serial.print(F("RÉSUMÉ Note "));
  Serial.println(NOTE_NAMES[_currentNoteIndex]);
  Serial.println(F("---------------"));
  Serial.print(F("MIDI: "));
  Serial.println(note.midiNote);
  Serial.print(F("Doigté: {"));
  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    Serial.print(note.fingerPattern[i]);
    if (i < NUMBER_SERVOS_FINGER - 1) Serial.print(F(","));
  }
  Serial.println(F("}"));
  Serial.print(F("airflowMinPercent: "));
  Serial.println(note.airflowMinPercent);
  Serial.print(F("airflowMaxPercent: "));
  Serial.println(note.airflowMaxPercent);
  Serial.println();

  Serial.print(F("Confirmer? (o/n): "));
  return waitForConfirmation();
}

void AirflowCalibrator::setAirflowPercent(byte percent) {
  uint16_t angle = percentToAngle(percent);
  setServoAngle(NUM_SERVO_AIRFLOW, angle);
}

uint16_t AirflowCalibrator::percentToAngle(byte percent) {
  if (percent > 100) percent = 100;
  return map(percent, 0, 100, SERVO_AIRFLOW_MIN, SERVO_AIRFLOW_MAX);
}

void AirflowCalibrator::openSolenoid() {
  pinMode(SOLENOID_PIN, OUTPUT);
  digitalWrite(SOLENOID_PIN, SOLENOID_ACTIVE_HIGH ? HIGH : LOW);
  Serial.println(F("  [Solénoïde ouvert]"));
}

void AirflowCalibrator::closeSolenoid() {
  pinMode(SOLENOID_PIN, OUTPUT);
  digitalWrite(SOLENOID_PIN, SOLENOID_ACTIVE_HIGH ? LOW : HIGH);
}

void AirflowCalibrator::adjustPercent(int delta) {
  int newPercent = (int)_currentPercent + delta;
  if (newPercent < 0) newPercent = 0;
  if (newPercent > 100) newPercent = 100;
  _currentPercent = (byte)newPercent;
}

void AirflowCalibrator::testNote(int durationMs) {
  Serial.print(F("[TEST] Note jouée pendant "));
  Serial.print(durationMs);
  Serial.print(F("ms à "));
  Serial.print(_currentPercent);
  Serial.println(F("%"));

  delay(durationMs);

  Serial.println(F("[TEST] Terminé"));
}

void AirflowCalibrator::setServoAngle(byte pcaChannel, uint16_t angle) {
  uint16_t pwm = angleToPWM(angle);
  _pwm.setPWM(pcaChannel, 0, pwm);
}

uint16_t AirflowCalibrator::angleToPWM(uint16_t angle) {
  return map(angle, 0, 180, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
}

bool AirflowCalibrator::waitForConfirmation() {
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
