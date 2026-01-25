#include "InstrumentManager.h"

InstrumentManager::InstrumentManager()
  : _pwm(Adafruit_PWMServoDriver()),
    _eventQueue(EVENT_QUEUE_SIZE),
    _fingerCtrl(_pwm),
    _airflowCtrl(_pwm),
    _sequencer(_eventQueue, _fingerCtrl, _airflowCtrl),
    _lastActivityTime(0),
    _servosPowered(false),
    _ccVolume(127),        // Volume max par défaut
    _ccExpression(127),    // Expression max par défaut
    _ccModulation(0) {     // Pas de modulation par défaut
}

void InstrumentManager::begin() {
  if (DEBUG) {
    Serial.println("DEBUG: InstrumentManager - Initialisation");
  }

  // Configurer le pin de contrôle d'alimentation des servos
  pinMode(PIN_SERVOS_OFF, OUTPUT);
  powerOnServos();

  // Initialiser le PCA9685
  _pwm.begin();
  _pwm.setPWMFreq(SERVO_FREQUENCY);

  // Vérifier la communication I2C
  delay(10);

  // Initialiser les contrôleurs
  _fingerCtrl.begin();
  _airflowCtrl.begin();

  // Initialiser les valeurs CC dans AirflowController
  _airflowCtrl.setCCValues(_ccVolume, _ccExpression, _ccModulation);

  // Initialiser le séquenceur
  _sequencer.begin();

  _lastActivityTime = millis();

  if (DEBUG) {
    Serial.println("DEBUG: InstrumentManager - Prêt");
  }
}

void InstrumentManager::update() {
  // Mettre à jour le séquenceur (state machine)
  _sequencer.update();

  // Mettre à jour le contrôleur d'air (gestion PWM solénoïde)
  _airflowCtrl.update();

  // Gérer l'alimentation des servos
  managePower();
}

void InstrumentManager::noteOn(byte midiNote, byte velocity) {
  // Vérifier si la note est jouable
  if (!isNotePlayable(midiNote)) {
    if (DEBUG) {
      Serial.print("DEBUG: InstrumentManager - Note hors plage: ");
      Serial.println(midiNote);
    }
    return;
  }

  // Ajouter l'événement à la queue avec timestamp actuel
  bool success = _eventQueue.enqueue(EVENT_NOTE_ON, midiNote, velocity, millis());

  if (!success) {
    if (DEBUG) {
      Serial.println("ERREUR: InstrumentManager - Queue pleine, événement perdu!");
    }
  } else {
    if (DEBUG) {
      Serial.print("DEBUG: InstrumentManager - Note On ajoutée: ");
      Serial.print(midiNote);
      Serial.print(" (vélocité: ");
      Serial.print(velocity);
      Serial.println(")");
    }
  }

  // Mise à jour de l'activité
  _lastActivityTime = millis();
}

void InstrumentManager::noteOff(byte midiNote) {
  // Ajouter l'événement à la queue avec timestamp actuel
  bool success = _eventQueue.enqueue(EVENT_NOTE_OFF, midiNote, 0, millis());

  if (!success) {
    if (DEBUG) {
      Serial.println("ERREUR: InstrumentManager - Queue pleine, événement perdu!");
    }
  } else {
    if (DEBUG) {
      Serial.print("DEBUG: InstrumentManager - Note Off ajoutée: ");
      Serial.println(midiNote);
    }
  }

  // Mise à jour de l'activité
  _lastActivityTime = millis();
}

bool InstrumentManager::isNotePlayable(byte midiNote) const {
  // Vérifier si la note existe dans le tableau NOTES
  return (getNoteByMidi(midiNote) != nullptr);
}

NoteSequencer& InstrumentManager::getSequencer() {
  return _sequencer;
}

void InstrumentManager::managePower() {
  // Si le séquenceur joue une note, garder l'alimentation
  if (_sequencer.isPlaying() || _sequencer.getState() != STATE_IDLE) {
    if (!_servosPowered) {
      powerOnServos();
    }
    _lastActivityTime = millis();
  } else {
    // Si inactif depuis TIMEUNPOWER ms, couper l'alimentation
    unsigned long elapsed = millis() - _lastActivityTime;
    if (elapsed >= TIMEUNPOWER && _servosPowered) {
      powerOffServos();
    }
  }
}

void InstrumentManager::powerOnServos() {
  digitalWrite(PIN_SERVOS_OFF, LOW);  // OE à LOW = servos activés
  _servosPowered = true;

  if (DEBUG) {
    Serial.println("DEBUG: InstrumentManager - Servos ACTIVÉS");
  }
}

void InstrumentManager::powerOffServos() {
  digitalWrite(PIN_SERVOS_OFF, HIGH);  // OE à HIGH = servos désactivés
  _servosPowered = false;

  if (DEBUG) {
    Serial.println("DEBUG: InstrumentManager - Servos DÉSACTIVÉS (anti-bruit)");
  }
}

void InstrumentManager::handleControlChange(byte ccNumber, byte ccValue) {
  switch (ccNumber) {
    case 1:  // Modulation
      _ccModulation = ccValue;
      _airflowCtrl.setCCValues(_ccVolume, _ccExpression, _ccModulation);
      if (DEBUG) {
        Serial.print("DEBUG: CC 1 (Modulation) = ");
        Serial.println(ccValue);
      }
      break;

    case 7:  // Volume
      _ccVolume = ccValue;
      _airflowCtrl.setCCValues(_ccVolume, _ccExpression, _ccModulation);
      if (DEBUG) {
        Serial.print("DEBUG: CC 7 (Volume) = ");
        Serial.println(ccValue);
      }
      break;

    case 11: // Expression
      _ccExpression = ccValue;
      _airflowCtrl.setCCValues(_ccVolume, _ccExpression, _ccModulation);
      if (DEBUG) {
        Serial.print("DEBUG: CC 11 (Expression) = ");
        Serial.println(ccValue);
      }
      break;

    case 120: // All Sound Off
      if (DEBUG) {
        Serial.println("DEBUG: CC 120 (All Sound Off)");
      }
      allSoundOff();
      break;

    default:
      // CC non supporté, ignorer silencieusement
      break;
  }
}

void InstrumentManager::allSoundOff() {
  // Vider la queue d'événements
  while (!_eventQueue.isEmpty()) {
    _eventQueue.dequeue();
  }

  // Stopper le séquenceur
  _sequencer.stop();

  // Fermer la valve et mettre airflow au repos
  _airflowCtrl.closeSolenoid();
  _airflowCtrl.setAirflowToRest();

  // Mettre tous les servos doigts en position repos (tous fermés)
  _fingerCtrl.closeAllFingers();

  if (DEBUG) {
    Serial.println("DEBUG: InstrumentManager - All Sound Off exécuté");
  }
}
