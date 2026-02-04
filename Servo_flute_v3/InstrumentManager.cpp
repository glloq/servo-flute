#include "InstrumentManager.h"

InstrumentManager::InstrumentManager()
  : _pwm(Adafruit_PWMServoDriver()),
    _eventQueue(EVENT_QUEUE_SIZE),
    _fingerCtrl(_pwm),
    _airflowCtrl(_pwm),
    _sequencer(_eventQueue, _fingerCtrl, _airflowCtrl),
    _lastActivityTime(0),
    _servosPowered(false),
    _ccVolume(CC_VOLUME_DEFAULT),
    _ccExpression(CC_EXPRESSION_DEFAULT),
    _ccModulation(CC_MODULATION_DEFAULT),
    _ccBreath(CC_BREATH_DEFAULT),
    _ccBrightness(CC_BRIGHTNESS_DEFAULT),
    _pitchBend(0),                    // Centre
    _lastCCTime(0),
    _ccCount(0),
    _ccWindowStart(0) {
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
  // Validation sécurité: ccValue doit être dans [0, 127]
  if (ccValue > 127) {
    if (DEBUG) {
      Serial.print("ERREUR: CC invalide - valeur hors range: ");
      Serial.println(ccValue);
    }
    return;  // Ignorer message invalide
  }

  // Rate limiting: Maximum CC_RATE_LIMIT_PER_SECOND CC par seconde
  unsigned long currentTime = millis();

  // Réinitialiser le compteur si nouvelle fenêtre d'une seconde
  if (currentTime - _ccWindowStart >= 1000) {
    _ccWindowStart = currentTime;
    _ccCount = 0;
  }

  // Vérifier le rate limit (sauf pour CC urgents: 120, 121, 123)
  if (ccNumber != 120 && ccNumber != 121 && ccNumber != 123) {
    _ccCount++;
    if (_ccCount > CC_RATE_LIMIT_PER_SECOND) {
      if (DEBUG) {
        Serial.println("ATTENTION: Rate limit CC dépassé, message ignoré");
      }
      return;  // Ignorer si rate limit dépassé
    }
  }

  _lastCCTime = currentTime;

  switch (ccNumber) {
    case 1:  // Modulation (Vibrato)
      _ccModulation = ccValue;
      _airflowCtrl.setCCValues(_ccVolume, _ccExpression, _ccModulation);
      if (DEBUG) {
        Serial.print("DEBUG: CC 1 (Modulation) = ");
        Serial.println(ccValue);
      }
      break;

    case 2:  // Breath Controller
      _ccBreath = ccValue;
      // Le breath controller peut moduler l'airflow comme l'expression
      // Pour l'instant, on le stocke pour utilisation future
      if (DEBUG) {
        Serial.print("DEBUG: CC 2 (Breath Controller) = ");
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

    case 74: // Brightness/Timbre
      _ccBrightness = ccValue;
      // La brightness pourrait moduler le vibrato ou l'airflow
      // Pour l'instant, on le stocke pour utilisation future
      if (DEBUG) {
        Serial.print("DEBUG: CC 74 (Brightness) = ");
        Serial.println(ccValue);
      }
      break;

    case 120: // All Sound Off
      if (DEBUG) {
        Serial.println("DEBUG: CC 120 (All Sound Off)");
      }
      allSoundOff();
      break;

    case 121: // Reset All Controllers
      if (DEBUG) {
        Serial.println("DEBUG: CC 121 (Reset All Controllers)");
      }
      resetAllControllers();
      break;

    case 123: // All Notes Off (même comportement que All Sound Off)
      if (DEBUG) {
        Serial.println("DEBUG: CC 123 (All Notes Off)");
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

void InstrumentManager::resetAllControllers() {
  // Réinitialiser tous les Control Changes à leurs valeurs par défaut
  _ccVolume = CC_VOLUME_DEFAULT;
  _ccExpression = CC_EXPRESSION_DEFAULT;
  _ccModulation = CC_MODULATION_DEFAULT;
  _ccBreath = CC_BREATH_DEFAULT;
  _ccBrightness = CC_BRIGHTNESS_DEFAULT;

  // Réinitialiser le pitch bend à 0 (centre)
  _pitchBend = 0;

  // Mettre à jour l'AirflowController
  _airflowCtrl.setCCValues(_ccVolume, _ccExpression, _ccModulation);

  if (DEBUG) {
    Serial.println("DEBUG: InstrumentManager - Reset All Controllers exécuté");
  }
}

void InstrumentManager::handlePitchBend(uint16_t pitchBendValue) {
  // Pitch bend MIDI: 0-16383, centre = 8192
  // Conversion en valeur signée: -8192 à +8191
  _pitchBend = (int16_t)pitchBendValue - 8192;

  // Le pitch bend module l'airflow de manière fine
  // Calcul du facteur de modulation: -1.0 à +1.0
  float pitchBendFactor = (float)_pitchBend / 8192.0;

  // Appliquer le pitch bend à l'airflow
  // Le pitch bend modifie l'airflow de ±PITCH_BEND_AIRFLOW_PERCENT%
  int8_t airflowAdjustment = (int8_t)(pitchBendFactor * PITCH_BEND_AIRFLOW_PERCENT);

  // Transmettre au contrôleur d'airflow
  _airflowCtrl.setPitchBendAdjustment(airflowAdjustment);

  if (DEBUG) {
    Serial.print("DEBUG: Pitch Bend = ");
    Serial.print(_pitchBend);
    Serial.print(" (raw: ");
    Serial.print(pitchBendValue);
    Serial.print(") | Ajustement airflow: ");
    Serial.print(airflowAdjustment);
    Serial.println("%");
  }
}
