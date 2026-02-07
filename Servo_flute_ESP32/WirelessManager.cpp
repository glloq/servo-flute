#include "WirelessManager.h"
#include "InstrumentManager.h"

WirelessManager::WirelessManager(StatusLed& led, HardwareInputs& inputs)
  : _led(led), _inputs(inputs), _instrument(nullptr),
    _currentMode(MODE_BLUETOOTH) {
}

void WirelessManager::begin(InstrumentManager* instrument) {
  _instrument = instrument;
  _currentMode = _inputs.getMode();

  if (DEBUG) {
    Serial.println("========================================");
    Serial.print("   MODE: ");
    Serial.println(_currentMode == MODE_BLUETOOTH ? "BLUETOOTH (BLE-MIDI)" : "WIFI (rtpMIDI)");
    Serial.println("========================================");
  }

  if (_currentMode == MODE_BLUETOOTH) {
    // Mode BLE-MIDI
    _bleMidi.begin(instrument);
    _led.setPattern(LED_BLINK_FAST);  // Advertising actif

  } else {
    // Mode WiFi - demarrer en AP par defaut
    // Phase 2 ajoutera la lecture de credentials WiFi depuis LittleFS
    // pour tenter une connexion STA d'abord
    _wifiMidi.begin(instrument);
    _led.setPattern(LED_TRIPLE_FLASH);  // Mode AP
  }
}

void WirelessManager::update() {
  // Lire les evenements bouton
  ButtonEvent event = _inputs.getButtonEvent();
  if (event != BUTTON_NONE) {
    handleButtonEvent(event);
  }

  // Mettre a jour le handler MIDI actif
  if (_currentMode == MODE_BLUETOOTH) {
    _bleMidi.update();
  } else {
    _wifiMidi.update();
  }

  // Mettre a jour le pattern LED
  updateLedPattern();
}

OperatingMode WirelessManager::getMode() const {
  return _currentMode;
}

bool WirelessManager::isMidiConnected() const {
  if (_currentMode == MODE_BLUETOOTH) {
    return _bleMidi.isConnected();
  } else {
    return _wifiMidi.isConnected();
  }
}

String WirelessManager::getStatusText() const {
  if (_currentMode == MODE_BLUETOOTH) {
    if (_bleMidi.isConnected()) {
      return "BLE: Connecte";
    } else if (_bleMidi.isAdvertising()) {
      return "BLE: Advertising...";
    }
    return "BLE: Inactif";
  } else {
    if (_wifiMidi.isAPMode()) {
      return "WiFi AP: " + _wifiMidi.getIPAddress();
    } else if (_wifiMidi.getState() == WIFI_STATE_STA_CONNECTED) {
      return "WiFi: " + _wifiMidi.getIPAddress();
    } else if (_wifiMidi.getState() == WIFI_STATE_CONNECTING) {
      return "WiFi: Connexion...";
    }
    return "WiFi: Deconnecte";
  }
}

void WirelessManager::handleButtonEvent(ButtonEvent event) {
  if (_currentMode == MODE_BLUETOOTH) {
    if (event == BUTTON_SHORT_PRESS) {
      // Relancer l'advertising BLE
      if (DEBUG) {
        Serial.println("DEBUG: WirelessManager - Restart advertising BLE");
      }
      _bleMidi.startAdvertising();
    }
    // Appui long en mode BT : pas d'action specifique pour l'instant
    if (event == BUTTON_LONG_PRESS) {
      if (DEBUG) {
        Serial.println("DEBUG: WirelessManager - Appui long en mode BT (ignore)");
      }
    }

  } else {
    // Mode WiFi
    if (event == BUTTON_SHORT_PRESS) {
      // Afficher l'IP sur le port serie
      if (DEBUG) {
        Serial.print("DEBUG: WirelessManager - IP: ");
        Serial.println(_wifiMidi.getIPAddress());
      }
    }

    if (event == BUTTON_LONG_PRESS) {
      // Forcer le mode AP (hotspot)
      if (DEBUG) {
        Serial.println("DEBUG: WirelessManager - Forcer mode AP (hotspot)");
      }
      _wifiMidi.forceAP();
    }
  }
}

void WirelessManager::updateLedPattern() {
  if (_currentMode == MODE_BLUETOOTH) {
    if (_bleMidi.isConnected()) {
      _led.setPattern(LED_BLINK_SLOW);     // Connecte : cligno lent
    } else {
      _led.setPattern(LED_BLINK_FAST);     // Advertising : cligno rapide
    }
  } else {
    switch (_wifiMidi.getState()) {
      case WIFI_STATE_CONNECTING:
        _led.setPattern(LED_BLINK_FAST);    // Connexion en cours
        break;
      case WIFI_STATE_STA_CONNECTED:
        _led.setPattern(LED_DOUBLE_FLASH);  // WiFi STA connecte
        break;
      case WIFI_STATE_AP_ACTIVE:
        _led.setPattern(LED_TRIPLE_FLASH);  // Mode hotspot
        break;
      default:
        _led.setPattern(LED_BLINK_FAST);    // Deconnecte
        break;
    }
  }
}
