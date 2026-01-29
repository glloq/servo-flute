/***********************************************************************************************
 * CALIBRATION MANAGER - IMPLEMENTATION
 ***********************************************************************************************/
#include "CalibrationManager.h"

CalibrationManager::CalibrationManager()
  : _pwm(Adafruit_PWMServoDriver()),
    _fingerCal(_pwm),
    _airflowCal(_pwm),
    _fingersCalibrated(false),
    _notesCalibrated(false) {
}

void CalibrationManager::begin() {
  // Initialiser Serial
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {
    // Attendre connexion Serial (max 3s)
  }

  printWelcomeBanner();

  // Initialiser I2C
  Serial.println(F("Initialisation I2C..."));
  Wire.begin();

  // Initialiser PCA9685
  Serial.println(F("Initialisation PCA9685..."));
  _pwm.begin();
  _pwm.setPWMFreq(SERVO_FREQUENCY);
  delay(10);

  // Initialiser les valeurs par défaut
  initializeDefaults();

  // Mettre tous les servos au repos
  Serial.println(F("Positionnement servos au repos..."));
  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    uint16_t pwm = map(90, 0, 180, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
    _pwm.setPWM(i, 0, pwm);
  }

  // Servo airflow au repos
  uint16_t pwm = map(SERVO_AIRFLOW_OFF, 0, 180, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
  _pwm.setPWM(NUM_SERVO_AIRFLOW, 0, pwm);

  // Solénoïde fermé
  pinMode(SOLENOID_PIN, OUTPUT);
  digitalWrite(SOLENOID_PIN, SOLENOID_ACTIVE_HIGH ? LOW : HIGH);

  Serial.println(F("✓ Initialisation terminée!"));
  Serial.println();

  printCalibrationStatus();
}

void CalibrationManager::run() {
  displayMainMenu();
  int choice = waitForMenuChoice();
  handleMenuChoice(choice);
}

void CalibrationManager::displayMainMenu() {
  Serial.println(F("\n========================================"));
  Serial.println(F("       MENU PRINCIPAL"));
  Serial.println(F("========================================"));
  Serial.println(F("1. Calibrer servos doigts (FINGERS)"));
  Serial.println(F("2. Calibrer plages airflow (NOTES)"));
  Serial.println(F("3. Afficher configuration actuelle"));
  Serial.println(F("4. Générer settings.h final"));
  Serial.println(F("========================================"));
  Serial.print(F("Votre choix (1-4): "));
}

void CalibrationManager::handleMenuChoice(int choice) {
  switch (choice) {
    case 1:
      calibrateAllFingers();
      break;

    case 2:
      if (!_fingersCalibrated) {
        Serial.println(F("\n⚠ ATTENTION: Vous devez calibrer les servos doigts d'abord!"));
        delay(2000);
      } else {
        calibrateAllNotes();
      }
      break;

    case 3:
      displayCurrentConfig();
      break;

    case 4:
      if (!_fingersCalibrated || !_notesCalibrated) {
        Serial.println(F("\n⚠ ATTENTION: Calibration incomplète!"));
        if (!_fingersCalibrated) {
          Serial.println(F("  - Servos doigts: NON calibrés"));
        }
        if (!_notesCalibrated) {
          Serial.println(F("  - Notes: NON calibrées"));
        }
        Serial.println(F("\nVoulez-vous générer quand même? (o/n): "));

        while (true) {
          while (Serial.available() == 0) { }
          char c = Serial.read();
          if (c == 'o' || c == 'O') {
            Serial.println(F("o"));
            generateOutput();
            break;
          } else if (c == 'n' || c == 'N') {
            Serial.println(F("n"));
            break;
          }
        }
      } else {
        generateOutput();
      }
      break;

    default:
      Serial.println(F("\n❌ Choix invalide!"));
      delay(1000);
      break;
  }

  printCalibrationStatus();
}

void CalibrationManager::calibrateAllFingers() {
  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("  CALIBRATION DES SERVOS DOIGTS"));
  Serial.println(F("========================================"));
  Serial.print(F("Nombre de doigts à calibrer: "));
  Serial.println(NUMBER_SERVOS_FINGER);
  Serial.println();
  Serial.println(F("Appuyez sur ENTRÉE pour commencer..."));

  while (Serial.available() == 0) { }
  while (Serial.available() > 0) Serial.read(); // Vider buffer

  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    _fingerCal.calibrateFinger(i, _calibratedFingers[i]);
    delay(500);
  }

  _fingersCalibrated = true;

  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("✓ TOUS LES SERVOS DOIGTS SONT CALIBRÉS!"));
  Serial.println(F("========================================"));
  delay(2000);
}

void CalibrationManager::calibrateAllNotes() {
  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("  CALIBRATION DES PLAGES AIRFLOW"));
  Serial.println(F("========================================"));
  Serial.print(F("Nombre de notes à calibrer: "));
  Serial.println(NUMBER_NOTES);
  Serial.println();
  Serial.println(F("⚠ IMPORTANT:"));
  Serial.println(F("  - Ayez l'instrument prêt"));
  Serial.println(F("  - Vérifiez l'alimentation air"));
  Serial.println(F("  - Testez chaque note soigneusement"));
  Serial.println();
  Serial.println(F("Appuyez sur ENTRÉE pour commencer..."));

  while (Serial.available() == 0) { }
  while (Serial.available() > 0) Serial.read(); // Vider buffer

  for (int i = 0; i < NUMBER_NOTES; i++) {
    _airflowCal.calibrateNote(i, _calibratedFingers, _calibratedNotes[i]);
    delay(500);
  }

  _notesCalibrated = true;

  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("✓ TOUTES LES NOTES SONT CALIBRÉES!"));
  Serial.println(F("========================================"));
  delay(2000);
}

void CalibrationManager::displayCurrentConfig() {
  _outputGen.displayCurrentConfig(_calibratedFingers, _calibratedNotes);
  Serial.println(F("\nAppuyez sur ENTRÉE pour continuer..."));
  while (Serial.available() == 0) { }
  while (Serial.available() > 0) Serial.read();
}

void CalibrationManager::generateOutput() {
  _outputGen.generateCppCode(_calibratedFingers, _calibratedNotes);

  Serial.println();
  Serial.println(F("Le code ci-dessus peut être copié directement dans"));
  Serial.println(F("le fichier settings.h de Servo_flute_v3."));
  Serial.println();
  Serial.println(F("Appuyez sur ENTRÉE pour continuer..."));

  while (Serial.available() == 0) { }
  while (Serial.available() > 0) Serial.read();
}

void CalibrationManager::initializeDefaults() {
  // Copier les templates par défaut
  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    _calibratedFingers[i] = FINGERS_TEMPLATE[i];
  }

  for (int i = 0; i < NUMBER_NOTES; i++) {
    _calibratedNotes[i] = NOTES_TEMPLATE[i];
  }
}

int CalibrationManager::waitForMenuChoice() {
  while (true) {
    while (Serial.available() == 0) {
      // Attendre
    }

    int choice = Serial.parseInt();

    // Vider le buffer
    while (Serial.available() > 0) {
      Serial.read();
    }

    if (choice >= 1 && choice <= 4) {
      Serial.println(choice);
      return choice;
    }
  }
}

void CalibrationManager::printWelcomeBanner() {
  Serial.println(F("\n\n"));
  Serial.println(F("========================================"));
  Serial.println(F("   CALIBRATION TOOL - SERVO FLUTE V3"));
  Serial.println(F("========================================"));
  Serial.println(F("Version: 1.0 MVP"));
  Serial.println(F("Interface: Serial Monitor"));
  Serial.println(F("========================================"));
  Serial.println();
  Serial.println(F("Configuration:"));
  Serial.print(F("  - Servos doigts: "));
  Serial.println(NUMBER_SERVOS_FINGER);
  Serial.print(F("  - Notes jouables: "));
  Serial.println(NUMBER_NOTES);
  Serial.print(F("  - Fréquence PWM: "));
  Serial.print(SERVO_FREQUENCY);
  Serial.println(F(" Hz"));
  Serial.println();
}

void CalibrationManager::printCalibrationStatus() {
  Serial.println(F("\n========================================"));
  Serial.println(F("  STATUT CALIBRATION"));
  Serial.println(F("========================================"));
  Serial.print(F("Servos doigts: "));
  if (_fingersCalibrated) {
    Serial.println(F("✓ CALIBRÉS"));
  } else {
    Serial.println(F("❌ NON calibrés"));
  }

  Serial.print(F("Notes: "));
  if (_notesCalibrated) {
    Serial.println(F("✓ CALIBRÉES"));
  } else {
    Serial.println(F("❌ NON calibrées"));
  }
  Serial.println(F("========================================"));
}
