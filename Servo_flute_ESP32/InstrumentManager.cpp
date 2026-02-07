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
    _lastCCTime(0),
    _ccCount(0),
    _ccWindowStart(0),
    _cc2Count(0),
    _cc2WindowStart(0) {
}

void InstrumentManager::begin() {
  if (DEBUG) {
    Serial.println("DEBUG: InstrumentManager - Initialisation");
  }

  // Configurer le pin de controle d'alimentation des servos
  pinMode(PIN_SERVOS_OFF, OUTPUT);
  powerOnServos();

  // Initialiser le PCA9685
  _pwm.begin();
  _pwm.setPWMFreq(SERVO_FREQUENCY);
  delay(10);

  // Initialiser les controleurs
  _fingerCtrl.begin();
  _airflowCtrl.begin();

  // Initialiser les valeurs CC dans AirflowController
  _airflowCtrl.setCCValues(_ccVolume, _ccExpression, _ccModulation);

  // Initialiser le sequenceur
  _sequencer.begin();

  _lastActivityTime = millis();

  if (DEBUG) {
    Serial.println("DEBUG: InstrumentManager - Pret");
  }
}

void InstrumentManager::update() {
  _sequencer.update();
  _airflowCtrl.update();
  managePower();
}

void InstrumentManager::noteOn(byte midiNote, byte velocity) {
  if (!isNotePlayable(midiNote)) {
    if (DEBUG) {
      Serial.print("DEBUG: InstrumentManager - Note hors plage: ");
      Serial.println(midiNote);
    }
    return;
  }

  bool success = _eventQueue.enqueue(EVENT_NOTE_ON, midiNote, velocity, millis());

  if (!success) {
    if (DEBUG) {
      Serial.println("ERREUR: InstrumentManager - Queue pleine!");
    }
  } else {
    if (DEBUG) {
      Serial.print("DEBUG: InstrumentManager - Note On: ");
      Serial.print(midiNote);
      Serial.print(" (vel: ");
      Serial.print(velocity);
      Serial.println(")");
    }
  }

  _lastActivityTime = millis();
}

void InstrumentManager::noteOff(byte midiNote) {
  bool success = _eventQueue.enqueue(EVENT_NOTE_OFF, midiNote, 0, millis());

  if (!success) {
    if (DEBUG) {
      Serial.println("ERREUR: InstrumentManager - Queue pleine!");
    }
  } else {
    if (DEBUG) {
      Serial.print("DEBUG: InstrumentManager - Note Off: ");
      Serial.println(midiNote);
    }
  }

  _lastActivityTime = millis();
}

bool InstrumentManager::isNotePlayable(byte midiNote) const {
  return (getNoteByMidi(midiNote) != nullptr);
}

NoteSequencer& InstrumentManager::getSequencer() {
  return _sequencer;
}

void InstrumentManager::managePower() {
  if (_sequencer.isPlaying() || _sequencer.getState() != STATE_IDLE) {
    if (!_servosPowered) {
      powerOnServos();
    }
    _lastActivityTime = millis();
  } else {
    unsigned long elapsed = millis() - _lastActivityTime;
    if (elapsed >= TIMEUNPOWER && _servosPowered) {
      powerOffServos();
    }
  }
}

void InstrumentManager::powerOnServos() {
  digitalWrite(PIN_SERVOS_OFF, LOW);  // OE a LOW = servos actives
  _servosPowered = true;

  if (DEBUG) {
    Serial.println("DEBUG: InstrumentManager - Servos ACTIVES");
  }
}

void InstrumentManager::powerOffServos() {
  digitalWrite(PIN_SERVOS_OFF, HIGH);  // OE a HIGH = servos desactives
  _servosPowered = false;

  if (DEBUG) {
    Serial.println("DEBUG: InstrumentManager - Servos DESACTIVES (anti-bruit)");
  }
}

void InstrumentManager::handleControlChange(byte ccNumber, byte ccValue) {
  if (ccValue > 127) {
    if (DEBUG) {
      Serial.print("ERREUR: CC invalide - valeur: ");
      Serial.println(ccValue);
    }
    return;
  }

  unsigned long currentTime = millis();

  // Rate limiting CC2 (Breath Controller) separe
  if (ccNumber == 2) {
    #if CC2_ENABLED
    if (currentTime - _cc2WindowStart >= 1000) {
      _cc2WindowStart = currentTime;
      _cc2Count = 0;
    }
    _cc2Count++;
    if (_cc2Count > CC2_RATE_LIMIT_PER_SECOND) {
      return;
    }
    #endif
  } else {
    // Rate limiting normal
    if (currentTime - _ccWindowStart >= 1000) {
      _ccWindowStart = currentTime;
      _ccCount = 0;
    }
    if (ccNumber != 120 && ccNumber != 121 && ccNumber != 123) {
      _ccCount++;
      if (_ccCount > CC_RATE_LIMIT_PER_SECOND) {
        return;
      }
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
      _airflowCtrl.updateCC2Breath(ccValue);
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

    case 74: // Brightness
      _ccBrightness = ccValue;
      break;

    case 120: // All Sound Off
      allSoundOff();
      break;

    case 121: // Reset All Controllers
      resetAllControllers();
      break;

    case 123: // All Notes Off
      allSoundOff();
      break;

    default:
      break;
  }
}

void InstrumentManager::allSoundOff() {
  while (!_eventQueue.isEmpty()) {
    _eventQueue.dequeue();
  }
  _sequencer.stop();
  _airflowCtrl.closeSolenoid();
  _airflowCtrl.setAirflowToRest();
  _fingerCtrl.closeAllFingers();

  if (DEBUG) {
    Serial.println("DEBUG: InstrumentManager - All Sound Off");
  }
}

void InstrumentManager::resetAllControllers() {
  _ccVolume = CC_VOLUME_DEFAULT;
  _ccExpression = CC_EXPRESSION_DEFAULT;
  _ccModulation = CC_MODULATION_DEFAULT;
  _ccBreath = CC_BREATH_DEFAULT;
  _ccBrightness = CC_BRIGHTNESS_DEFAULT;
  _airflowCtrl.setCCValues(_ccVolume, _ccExpression, _ccModulation);

  if (DEBUG) {
    Serial.println("DEBUG: InstrumentManager - Reset All Controllers");
  }
}
