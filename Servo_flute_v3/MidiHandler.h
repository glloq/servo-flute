#ifndef MIDI_HANDLER_H
#define MIDI_HANDLER_H

#include <Arduino.h>
#include <MIDIUSB.h>
#include "InstrumentManager.h"
#include "settings.h"

class MidiHandler {
public:
  MidiHandler(InstrumentManager& instrument);

  // Lit et traite les événements MIDI depuis USB
  void readMidi();

private:
  InstrumentManager& _instrument;

  // Traite un événement MIDI reçu
  void processMidiEvent(midiEventPacket_t midiEvent);

  // Vérifie si le message MIDI doit être traité selon le canal configuré
  bool isChannelAccepted(byte channel);
};

#endif
