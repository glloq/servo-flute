#include "NoteSequencer.h"

NoteSequencer::NoteSequencer(EventQueue& eventQueue, FingerController& fingerCtrl, AirflowController& airflowCtrl)
  : _eventQueue(eventQueue), _fingerCtrl(fingerCtrl), _airflowCtrl(airflowCtrl),
    _currentState(STATE_IDLE), _currentNote(0), _currentVelocity(0),
    _stateStartTime(0), _eventScheduledTime(0), _playbackStartTime(0) {
}

void NoteSequencer::begin() {
  _currentState = STATE_IDLE;

  if (DEBUG) {
    Serial.println("DEBUG: NoteSequencer - Initialisation");
  }
}

void NoteSequencer::update() {
  // Dispatcher selon l'état actuel
  switch (_currentState) {
    case STATE_IDLE:
      handleIdle();
      break;
    case STATE_POSITIONING:
      handlePositioning();
      break;
    case STATE_PLAYING:
      handlePlaying();
      break;
    case STATE_STOPPING:
      handleStopping();
      break;
  }
}

NoteState NoteSequencer::getState() const {
  return _currentState;
}

bool NoteSequencer::isPlaying() const {
  return _currentState == STATE_PLAYING;
}

void NoteSequencer::handleIdle() {
  // Si la queue n'est pas vide, traiter le prochain événement
  if (!_eventQueue.isEmpty()) {
    processNextEvent();
  }
}

void NoteSequencer::handlePositioning() {
  // Vérifier si le délai total est écoulé (servos + stabilisation)
  unsigned long elapsed = millis() - _stateStartTime;

  if (elapsed >= SERVO_TO_SOLENOID_DELAY_MS) {
    // Activer le servo de débit selon la note et la vélocité
    _airflowCtrl.setAirflowForNote(_currentNote, _currentVelocity);

    // Ouvrir le solénoïde -> SON PRODUIT
    _airflowCtrl.openSolenoid();

    // Transition vers état PLAYING
    transitionTo(STATE_PLAYING);

    if (DEBUG) {
      unsigned long actualTime = millis() - _playbackStartTime;
      unsigned long targetTime = _eventScheduledTime - _playbackStartTime;
      long timing_error = (long)actualTime - (long)targetTime;

      Serial.print("DEBUG: NoteSequencer - 🎵 SON produit note ");
      Serial.print(_currentNote);
      Serial.print(" (vel: ");
      Serial.print(_currentVelocity);
      Serial.print(") | t=");
      Serial.print(actualTime);
      Serial.print("ms | Cible: ");
      Serial.print(targetTime);
      Serial.print("ms | Erreur: ");
      Serial.print(timing_error);
      Serial.println("ms");
    }
  }
}

void NoteSequencer::handlePlaying() {
  // Vérifier s'il y a un noteOff dans la queue pour cette note
  MidiEvent* nextEvent = _eventQueue.peek();

  if (nextEvent != nullptr && nextEvent->type == EVENT_NOTE_OFF &&
      nextEvent->midiNote == _currentNote) {

    // Vérifier si le timing du noteOff est atteint
    unsigned long eventAbsoluteTime = _playbackStartTime + nextEvent->timestamp;

    if (millis() >= eventAbsoluteTime) {
      // Retirer l'événement de la queue
      _eventQueue.dequeue();

      // Arrêter la note
      stopCurrentNote();
    }
  }
}

void NoteSequencer::handleStopping() {
  // Transition immédiate vers IDLE
  transitionTo(STATE_IDLE);
}

void NoteSequencer::processNextEvent() {
  MidiEvent* event = _eventQueue.peek();

  if (event == nullptr) {
    return;
  }

  // Calculer le timestamp absolu de l'événement
  unsigned long eventAbsoluteTime = _playbackStartTime + event->timestamp;

  // Si c'est le premier événement, initialiser le playback start time
  if (_playbackStartTime == 0) {
    _playbackStartTime = millis();
    eventAbsoluteTime = _playbackStartTime + event->timestamp;
  }

  // ANTICIPATION : Calculer le délai mécanique total
  const unsigned long MECHANICAL_DELAY = SERVO_TO_SOLENOID_DELAY_MS;

  // Pour les NoteOn : démarrer la séquence en avance pour compenser le délai mécanique
  // Pour les NoteOff : exécuter au timing exact
  unsigned long startTime;
  if (event->type == EVENT_NOTE_ON) {
    // Anticiper : démarrer MECHANICAL_DELAY ms avant le timing prévu
    if (eventAbsoluteTime > MECHANICAL_DELAY) {
      startTime = eventAbsoluteTime - MECHANICAL_DELAY;
    } else {
      startTime = 0;  // Impossible d'anticiper, démarrer immédiatement
    }
  } else {
    // NoteOff : timing exact
    startTime = eventAbsoluteTime;
  }

  // Vérifier si le moment de démarrer est atteint
  if (millis() >= startTime) {
    // Traiter l'événement selon son type
    if (event->type == EVENT_NOTE_ON) {
      // Si une note est déjà en cours, l'arrêter d'abord
      if (_currentState != STATE_IDLE) {
        stopCurrentNote();
      }

      // Démarrer la nouvelle note (le son sera produit à eventAbsoluteTime)
      startNoteSequence(event->midiNote, event->velocity, eventAbsoluteTime);

      // Retirer l'événement de la queue
      _eventQueue.dequeue();

    } else if (event->type == EVENT_NOTE_OFF) {
      // Si c'est un noteOff sans note en cours, l'ignorer
      if (_currentState == STATE_PLAYING && event->midiNote == _currentNote) {
        stopCurrentNote();
      }

      // Retirer l'événement de la queue
      _eventQueue.dequeue();
    }
  }
  // Sinon, attendre que le timing soit atteint
}

void NoteSequencer::startNoteSequence(byte note, byte velocity, unsigned long scheduledTime) {
  _currentNote = note;
  _currentVelocity = velocity;
  _eventScheduledTime = scheduledTime;

  // Positionner les servos doigts
  _fingerCtrl.setFingerPatternForNote(note);

  // Transition vers POSITIONING
  transitionTo(STATE_POSITIONING);

  if (DEBUG) {
    unsigned long currentTime = millis() - _playbackStartTime;
    unsigned long targetTime = scheduledTime - _playbackStartTime;
    Serial.print("DEBUG: NoteSequencer - Début séquence note: ");
    Serial.print(note);
    Serial.print(" | Démarrage à t=");
    Serial.print(currentTime);
    Serial.print("ms | Son prévu à t=");
    Serial.print(targetTime);
    Serial.print("ms (dans ");
    Serial.print(SERVO_TO_SOLENOID_DELAY_MS);
    Serial.println("ms)");
  }
}

bool NoteSequencer::shouldCloseValveBetweenNotes() {
  // Regarder la prochaine note dans la queue
  MidiEvent* nextEvent = _eventQueue.peek();

  if (nextEvent == nullptr || nextEvent->type != EVENT_NOTE_ON) {
    // Pas de note suivante, fermer la valve
    return true;
  }

  // Calculer le temps jusqu'à la prochaine note
  unsigned long currentTime = millis();
  unsigned long nextNoteTime = _playbackStartTime + nextEvent->timestamp;

  // Si la prochaine note doit démarrer dans moins de MIN_NOTE_INTERVAL_FOR_VALVE_CLOSE_MS
  // on garde la valve ouverte pour économiser usure et améliorer fluidité
  if (nextNoteTime > currentTime) {
    unsigned long interval = nextNoteTime - currentTime;

    if (interval < MIN_NOTE_INTERVAL_FOR_VALVE_CLOSE_MS) {
      if (DEBUG) {
        Serial.print("DEBUG: NoteSequencer - Valve GARDÉE ouverte (note suivante dans ");
        Serial.print(interval);
        Serial.println("ms)");
      }
      return false;  // Ne pas fermer la valve
    }
  }

  return true;  // Fermer la valve
}

void NoteSequencer::stopCurrentNote() {
  // Vérifier s'il faut fermer la valve ou la garder ouverte
  bool closeValve = shouldCloseValveBetweenNotes();

  if (closeValve) {
    // Fermer le solénoïde
    _airflowCtrl.closeSolenoid();

    // Optionnel: remettre le servo de débit en position repos
    _airflowCtrl.setAirflowToRest();

    if (DEBUG) {
      Serial.print("DEBUG: NoteSequencer - Arrêt note: ");
      Serial.print(_currentNote);
      Serial.println(" (valve fermée)");
    }
  } else {
    // Garder la valve ouverte mais réduire le débit
    _airflowCtrl.setAirflowVelocity(1);  // Débit minimal

    if (DEBUG) {
      Serial.print("DEBUG: NoteSequencer - Arrêt note: ");
      Serial.print(_currentNote);
      Serial.println(" (valve OUVERTE, débit minimal)");
    }
  }

  // Transition vers STOPPING
  transitionTo(STATE_STOPPING);
}

void NoteSequencer::transitionTo(NoteState newState) {
  _currentState = newState;
  _stateStartTime = millis();

  if (DEBUG) {
    Serial.print("DEBUG: NoteSequencer - Transition vers état: ");
    Serial.println(newState);
  }
}
