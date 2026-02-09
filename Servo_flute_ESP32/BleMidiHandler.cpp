#include "BleMidiHandler.h"
#include "InstrumentManager.h"
#include "ConfigStorage.h"

// BLE-MIDI avec backend NimBLE
#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>

// Instance MIDI BLE globale
BLEMIDI_CREATE_INSTANCE(DEVICE_NAME, bleMidi);

// Instance statique pour les callbacks
BleMidiHandler* BleMidiHandler::_instance = nullptr;

BleMidiHandler::BleMidiHandler()
  : _instrument(nullptr), _connected(false), _advertising(false) {
  _instance = this;
}

void BleMidiHandler::begin(InstrumentManager* instrument) {
  _instrument = instrument;

  if (DEBUG) {
    Serial.println("DEBUG: BleMidiHandler - Initialisation BLE-MIDI");
    Serial.print("DEBUG:   Nom: ");
    Serial.println(DEVICE_NAME);
  }

  // Configurer les callbacks MIDI
  MIDI.setHandleNoteOn(onNoteOn);
  MIDI.setHandleNoteOff(onNoteOff);
  MIDI.setHandleControlChange(onControlChange);

  // Configurer les callbacks de connexion BLE
  BLEMIDI.setHandleConnected(onConnected);
  BLEMIDI.setHandleDisconnected(onDisconnected);

  // Demarrer MIDI (ecoute tous les canaux, filtrage fait dans les callbacks)
  MIDI.begin(MIDI_CHANNEL_OMNI);

  _advertising = true;

  if (DEBUG) {
    Serial.println("DEBUG: BleMidiHandler - BLE-MIDI pret, advertising actif");
  }
}

void BleMidiHandler::update() {
  // Lire les messages MIDI BLE entrants (non-bloquant)
  MIDI.read();
}

void BleMidiHandler::startAdvertising() {
  if (!_advertising) {
    // NimBLE redemarrage advertising
    BLEMIDI.enableDebugging(Serial);
    _advertising = true;

    if (DEBUG) {
      Serial.println("DEBUG: BleMidiHandler - Advertising demarre");
    }
  }
}

void BleMidiHandler::stopAdvertising() {
  _advertising = false;

  if (DEBUG) {
    Serial.println("DEBUG: BleMidiHandler - Advertising arrete");
  }
}

bool BleMidiHandler::isConnected() const {
  return _connected;
}

bool BleMidiHandler::isAdvertising() const {
  return _advertising && !_connected;
}

// --- Callbacks statiques ---

void BleMidiHandler::onNoteOn(byte channel, byte note, byte velocity) {
  if (_instance == nullptr || _instance->_instrument == nullptr) return;
  if (!_instance->isChannelAccepted(channel)) return;

  if (velocity > 0) {
    _instance->_instrument->noteOn(note, velocity);
  } else {
    _instance->_instrument->noteOff(note);
  }
}

void BleMidiHandler::onNoteOff(byte channel, byte note, byte velocity) {
  if (_instance == nullptr || _instance->_instrument == nullptr) return;
  if (!_instance->isChannelAccepted(channel)) return;

  _instance->_instrument->noteOff(note);
}

void BleMidiHandler::onControlChange(byte channel, byte number, byte value) {
  if (_instance == nullptr || _instance->_instrument == nullptr) return;
  if (!_instance->isChannelAccepted(channel)) return;

  _instance->_instrument->handleControlChange(number, value);
}

void BleMidiHandler::onConnected() {
  if (_instance == nullptr) return;
  _instance->_connected = true;

  if (DEBUG) {
    Serial.println("DEBUG: BleMidiHandler - Client BLE connecte");
  }
}

void BleMidiHandler::onDisconnected() {
  if (_instance == nullptr) return;
  _instance->_connected = false;

  if (DEBUG) {
    Serial.println("DEBUG: BleMidiHandler - Client BLE deconnecte");
  }
}

bool BleMidiHandler::isChannelAccepted(byte channel) {
  if (cfg.midiChannel == 0) return true;
  return (channel == cfg.midiChannel);
}
