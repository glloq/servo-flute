#include "MidiHandler.h"

MidiHandler::MidiHandler(InstrumentManager& instrument)
  : _instrument(instrument) {
  if (DEBUG) {
    Serial.println("DEBUG: MidiHandler - Création");
  }
}

void MidiHandler::readMidi() {
  midiEventPacket_t midiEvent;

  // Lire tous les événements MIDI en attente (non-bloquant)
  do {
    midiEvent = MidiUSB.read();
    if (midiEvent.header != 0) {
      processMidiEvent(midiEvent);
    }
  } while (midiEvent.header != 0);
}

void MidiHandler::processMidiEvent(midiEventPacket_t midiEvent) {
  byte messageType = midiEvent.byte1 & 0xF0;
  byte channel = midiEvent.byte1 & 0x0F;
  byte note = midiEvent.byte2;
  byte velocity = midiEvent.byte3;

  switch (messageType) {
    case 0x90:  // Note On
      if (velocity > 0) {
        _instrument.noteOn(note, velocity);
      } else {
        // Velocity 0 = Note Off
        _instrument.noteOff(note);
      }
      break;

    case 0x80:  // Note Off
      _instrument.noteOff(note);
      break;

    case 0xE0:  // Pitch Bend
      // Non implémenté pour l'instant
      break;

    case 0xA0:  // Channel Pressure (Aftertouch)
      // Non implémenté pour l'instant
      break;

    case 0xD0:  // Polyphonic Key Pressure
      // Non implémenté pour l'instant
      break;

    case 0xB0:  // Control Change
      {
        byte ccNumber = midiEvent.byte2;
        byte ccValue = midiEvent.byte3;
        _instrument.handleControlChange(ccNumber, ccValue);
      }
      break;

    case 0xF0:  // System Common or System Real-Time
      // Non implémenté pour l'instant
      break;

    default:
      break;
  }
}
