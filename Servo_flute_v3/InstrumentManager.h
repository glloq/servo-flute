#ifndef INSTRUMENT_MANAGER_H
#define INSTRUMENT_MANAGER_H

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include "EventQueue.h"
#include "FingerController.h"
#include "AirflowController.h"
#include "NoteSequencer.h"
#include "settings.h"

class InstrumentManager {
public:
  InstrumentManager();

  // Initialise tous les composants
  void begin();

  // Méthode update appelée dans loop()
  void update();

  // Ajoute un événement Note On à la queue
  void noteOn(byte midiNote, byte velocity);

  // Ajoute un événement Note Off à la queue
  void noteOff(byte midiNote);

  // Vérifie si une note est dans la plage jouable
  bool isNotePlayable(byte midiNote) const;

  // Retourne le séquenceur (pour debug/monitoring)
  NoteSequencer& getSequencer();

  // Gère les Control Change MIDI
  void handleControlChange(byte ccNumber, byte ccValue);

  // Accesseurs pour les valeurs CC (pour AirflowController)
  byte getCCVolume() const { return _ccVolume; }
  byte getCCExpression() const { return _ccExpression; }
  byte getCCModulation() const { return _ccModulation; }

  // All Sound Off
  void allSoundOff();

private:
  Adafruit_PWMServoDriver _pwm;
  EventQueue _eventQueue;
  FingerController _fingerCtrl;
  AirflowController _airflowCtrl;
  NoteSequencer _sequencer;

  unsigned long _lastActivityTime;
  bool _servosPowered;

  // Valeurs Control Change MIDI
  byte _ccVolume;       // CC 7  (défaut: 127 = 100%)
  byte _ccExpression;   // CC 11 (défaut: 127 = 100%)
  byte _ccModulation;   // CC 1  (défaut: 0 = pas de modulation)

  // Gère l'alimentation des servos (power management)
  void managePower();

  // Active l'alimentation des servos
  void powerOnServos();

  // Coupe l'alimentation des servos (anti-bruit)
  void powerOffServos();
};

#endif
