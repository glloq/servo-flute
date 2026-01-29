/***********************************************************************************************
 * SERVO FLUTE V3 - Automated Recorder Player
 *
 * Architecture améliorée avec :
 * - Mode binaire uniquement (ouvert/fermé, pas de demi-ouvert)
 * - Contrôle d'air hybride : servo débit + solénoïde valve
 * - Gestion du timing non-bloquante avec EventQueue
 * - Respect des délais relatifs entre notes MIDI
 * - State machine pour séquencement précis
 *
 * Hardware:
 * - Arduino Leonardo/Micro (USB MIDI natif)
 * - PCA9685 PWM Driver (I2C)
 * - 10 servos SG90 pour les doigts
 * - 1 servo pour le débit d'air
 * - 1 solénoïde pour valve on/off
 *
 * Auteur: Servo-Flute Project
 * Version: 3.0
 * Date: 2026-01-23
 ***********************************************************************************************/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <MIDIUSB.h>
#include <avr/wdt.h>  // Watchdog timer pour sécurité (P23)

#include "settings.h"
#include "EventQueue.h"
#include "FingerController.h"
#include "AirflowController.h"
#include "NoteSequencer.h"
#include "InstrumentManager.h"
#include "MidiHandler.h"

// Instances globales
InstrumentManager* instrument = nullptr;
MidiHandler* midiHandler = nullptr;

/**
 * P17 - État sûr en cas de crash/redémarrage
 * Initialise le hardware en configuration sûre AVANT toute autre opération.
 * Appelé au tout début du setup() pour garantir un état sûr même après
 * un reset watchdog ou une erreur système.
 */
void initSafeState() {
  // Initialiser I2C pour accéder au PCA9685
  Wire.begin();

  // Créer une instance temporaire du PWM driver (adresse par défaut 0x40)
  Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
  pwm.begin();
  pwm.setPWMFreq(SERVO_FREQUENCY);

  // Mettre le solénoïde en état sûr (FERMÉ)
  pinMode(SOLENOID_PIN, OUTPUT);
  digitalWrite(SOLENOID_PIN, LOW);  // Solénoïde fermé

  // Mettre le servo airflow en position repos
  uint16_t pulseWidth = map(SERVO_AIRFLOW_OFF, 0, 180, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
  pwm.setPWM(NUM_SERVO_AIRFLOW, 0, pulseWidth);

  // Mettre tous les servos doigts en position fermée (sécuritaire)
  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    pulseWidth = map(FINGERS[i].closedAngle, 0, 180, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
    pwm.setPWM(i, 0, pulseWidth);
  }

  // Petit délai pour que les servos atteignent la position
  delay(100);
}

void setup() {
  // P17 - Forcer l'état sûr dès le démarrage (protection crash/watchdog)
  initSafeState();
  // Initialiser la communication série pour debug
  if (DEBUG) {
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {
      ; // Attendre l'ouverture du port série (max 3s)
    }
    Serial.println("========================================");
    Serial.println("   SERVO FLUTE V3 - INITIALISATION");
    Serial.println("========================================");
  }

  // I2C déjà initialisé dans initSafeState()

  // Créer l'instrument manager
  instrument = new InstrumentManager();
  instrument->begin();

  // Créer le MIDI handler
  midiHandler = new MidiHandler(*instrument);

  if (DEBUG) {
    Serial.println("========================================");
    Serial.println("   SYSTÈME PRÊT - EN ATTENTE MIDI");
    Serial.println("========================================");
    Serial.println();
    Serial.println("Configuration:");
    Serial.print("  - Notes jouables: ");
    Serial.print(NUMBER_NOTES);
    Serial.print(" (MIDI ");
    Serial.print(FIRST_MIDI_NOTE);
    Serial.print(" - ");
    Serial.print(FIRST_MIDI_NOTE + NUMBER_NOTES - 1);
    Serial.println(")");
    Serial.print("  - Servos doigts: ");
    Serial.println(NUMBER_SERVOS_FINGER);
    Serial.print("  - Délai servos→solénoïde: ");
    Serial.print(SERVO_TO_SOLENOID_DELAY_MS);
    Serial.println(" ms");
    Serial.print("  - Taille queue: ");
    Serial.print(EVENT_QUEUE_SIZE);
    Serial.println(" événements");
    Serial.print("  - Watchdog: ");
    Serial.println("4 secondes (P23)");
    Serial.println();
  }

  // P23 - Activer le watchdog timer (timeout 4 secondes)
  // En cas de blocage, le système redémarre automatiquement en état sûr
  wdt_enable(WDTO_4S);

  if (DEBUG) {
    Serial.println("DEBUG: Watchdog activé (timeout 4s)");
  }
}

void loop() {
  // P23 - Réinitialiser le watchdog timer à chaque itération
  // Si la loop() se bloque, le watchdog redémarre le système après 4s
  wdt_reset();

  // Lire les événements MIDI entrants (non-bloquant)
  midiHandler->readMidi();

  // Mettre à jour l'instrument (state machine + power management)
  instrument->update();

  // Pas de delay() pour garder la boucle réactive
}
