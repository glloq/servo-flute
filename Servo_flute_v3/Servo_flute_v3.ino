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

void setup() {
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

  // Initialiser I2C
  Wire.begin();

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
    Serial.print("  - Délai servos: ");
    Serial.print(SERVO_SETTLE_TIME_MS);
    Serial.println(" ms");
    Serial.print("  - Taille queue: ");
    Serial.print(EVENT_QUEUE_SIZE);
    Serial.println(" événements");
    Serial.println();
  }
}

void loop() {
  // Lire les événements MIDI entrants (non-bloquant)
  midiHandler->readMidi();

  // Mettre à jour l'instrument (state machine + power management)
  instrument->update();

  // Pas de delay() pour garder la boucle réactive
}
