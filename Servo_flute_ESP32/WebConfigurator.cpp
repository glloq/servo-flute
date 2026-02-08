#include "WebConfigurator.h"
#include "InstrumentManager.h"
#include "WirelessManager.h"
#include "web_content.h"

WebConfigurator::WebConfigurator(uint16_t port)
  : _server(port), _ws("/ws"),
    _instrument(nullptr), _player(nullptr), _wirelessManager(nullptr),
    _webVelocity(100), _lastStatusBroadcast(0), _lastWsCleanup(0),
    _uploadSize(0) {
}

WebConfigurator::~WebConfigurator() {
}

void WebConfigurator::begin(InstrumentManager* instrument, MidiFilePlayer* player) {
  _instrument = instrument;
  _player = player;

  // Configurer le WebSocket
  _ws.onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client,
                     AwsEventType type, void* arg, uint8_t* data, size_t len) {
    onWsEvent(server, client, type, arg, data, len);
  });
  _server.addHandler(&_ws);

  // Configurer les routes HTTP
  setupRoutes();

  // Demarrer le serveur
  _server.begin();

  if (DEBUG) {
    Serial.println("DEBUG: WebConfigurator - Serveur web demarre");
  }
}

void WebConfigurator::update() {
  unsigned long now = millis();

  // Nettoyage periodique des clients WS deconnectes
  if (now - _lastWsCleanup >= WS_CLEANUP_INTERVAL_MS) {
    _ws.cleanupClients(WS_MAX_CLIENTS);
    _lastWsCleanup = now;
  }

  // Broadcast status periodique
  if (now - _lastStatusBroadcast >= WS_STATUS_INTERVAL_MS) {
    if (_ws.count() > 0) {
      broadcastStatus();
    }
    _lastStatusBroadcast = now;
  }
}

void WebConfigurator::setWirelessManager(WirelessManager* wm) {
  _wirelessManager = wm;
}

void WebConfigurator::setupRoutes() {
  // Page principale
  _server.on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleRoot(request);
  });

  // API Status
  _server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleApiStatus(request);
  });

  // API Config GET
  _server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleApiConfig(request);
  });

  // Upload MIDI (multipart)
  _server.on("/api/midi", HTTP_POST,
    [this](AsyncWebServerRequest* request) {
      handleMidiUploadComplete(request);
    },
    [this](AsyncWebServerRequest* request, const String& filename,
           size_t index, uint8_t* data, size_t len, bool final) {
      handleMidiUpload(request, filename, index, data, len, final);
    }
  );

  // 404
  _server.onNotFound([](AsyncWebServerRequest* request) {
    request->send(404, "text/plain", "Not found");
  });
}

void WebConfigurator::handleRoot(AsyncWebServerRequest* request) {
  request->send_P(200, "text/html", WEB_HTML_CONTENT);
}

void WebConfigurator::handleApiStatus(AsyncWebServerRequest* request) {
  String json = "{";
  json += "\"mode\":\"" + String(_wirelessManager ? _wirelessManager->getStatusText() : "N/A") + "\"";
  json += ",\"connected\":" + String(_wirelessManager ? (_wirelessManager->isMidiConnected() ? "true" : "false") : "false");
  json += ",\"uptime\":" + String(millis() / 1000);

  // CC values
  if (_instrument) {
    json += ",\"cc7\":" + String(_instrument->getCCVolume());
    json += ",\"cc11\":" + String(_instrument->getCCExpression());
    json += ",\"cc1\":" + String(_instrument->getCCModulation());
    json += ",\"cc2\":" + String(_instrument->getCCBreath());
  }

  // Player state
  if (_player) {
    json += ",\"player\":{";
    json += "\"state\":" + String(_player->getState());
    json += ",\"loaded\":" + String(_player->isFileLoaded() ? "true" : "false");
    json += ",\"file\":\"" + _player->getFileName() + "\"";
    json += ",\"events\":" + String(_player->getEventCount());
    json += ",\"duration\":" + String(_player->getDurationMs());
    json += ",\"position\":" + String(_player->getPositionMs());
    json += ",\"progress\":" + String(_player->getProgressPercent(), 1);
    json += "}";
  }

  json += "}";
  request->send(200, "application/json", json);
}

void WebConfigurator::handleApiConfig(AsyncWebServerRequest* request) {
  String json = "{";
  json += "\"midi_channel\":" + String(MIDI_CHANNEL);
  json += ",\"servo_solenoid_delay\":" + String(SERVO_TO_SOLENOID_DELAY_MS);
  json += ",\"num_notes\":" + String(NUMBER_NOTES);
  json += ",\"num_fingers\":" + String(NUMBER_SERVOS_FINGER);
  json += ",\"airflow_min\":" + String(SERVO_AIRFLOW_MIN);
  json += ",\"airflow_max\":" + String(SERVO_AIRFLOW_MAX);
  json += ",\"vibrato_freq\":" + String(VIBRATO_FREQUENCY_HZ, 1);
  json += ",\"vibrato_amp\":" + String(VIBRATO_MAX_AMPLITUDE_DEG, 1);
  json += ",\"cc2_enabled\":" + String(CC2_ENABLED ? "true" : "false");
  json += ",\"cc2_threshold\":" + String(CC2_SILENCE_THRESHOLD);
  json += ",\"cc2_curve\":" + String(CC2_RESPONSE_CURVE, 2);
  json += ",\"device_name\":\"" + String(DEVICE_NAME) + "\"";

  // Notes jouables
  json += ",\"notes\":[";
  for (int i = 0; i < NUMBER_NOTES; i++) {
    if (i > 0) json += ",";
    json += "{\"midi\":" + String(NOTES[i].midiNote);
    json += ",\"air_min\":" + String(NOTES[i].airflowMinPercent);
    json += ",\"air_max\":" + String(NOTES[i].airflowMaxPercent);
    json += ",\"fingers\":[";
    for (int f = 0; f < NUMBER_SERVOS_FINGER; f++) {
      if (f > 0) json += ",";
      json += String(NOTES[i].fingerPattern[f] ? 1 : 0);
    }
    json += "]}";
  }
  json += "]";

  json += "}";
  request->send(200, "application/json", json);
}

void WebConfigurator::handleMidiUpload(AsyncWebServerRequest* request, const String& filename,
                                        size_t index, uint8_t* data, size_t len, bool final) {
  if (index == 0) {
    // Debut de l'upload
    if (DEBUG) {
      Serial.print("DEBUG: WebConfigurator - Upload MIDI: ");
      Serial.println(filename);
    }
    _uploadSize = 0;
    _uploadFile = LittleFS.open(MIDI_FILE_PATH, "w");
    if (!_uploadFile) {
      if (DEBUG) {
        Serial.println("ERREUR: WebConfigurator - Impossible de creer fichier temp");
      }
      return;
    }
  }

  if (_uploadFile && len > 0) {
    _uploadSize += len;
    if (_uploadSize <= MIDI_FILE_MAX_SIZE) {
      _uploadFile.write(data, len);
    }
  }

  if (final) {
    if (_uploadFile) {
      _uploadFile.close();
    }

    if (DEBUG) {
      Serial.print("DEBUG: WebConfigurator - Upload termine: ");
      Serial.print(_uploadSize);
      Serial.println(" octets");
    }
  }
}

void WebConfigurator::handleMidiUploadComplete(AsyncWebServerRequest* request) {
  if (_uploadSize > MIDI_FILE_MAX_SIZE) {
    LittleFS.remove(MIDI_FILE_PATH);
    String resp = "{\"ok\":false,\"msg\":\"Fichier trop volumineux (max 100KB)\"}";
    request->send(400, "application/json", resp);

    // Notifier les clients WS
    _ws.textAll("{\"t\":\"midi_error\",\"msg\":\"Fichier trop volumineux\"}");
    return;
  }

  if (_uploadSize == 0) {
    String resp = "{\"ok\":false,\"msg\":\"Aucun fichier recu\"}";
    request->send(400, "application/json", resp);
    return;
  }

  // Parser le fichier
  if (_player && _player->loadFile(MIDI_FILE_PATH)) {
    String resp = "{\"ok\":true";
    resp += ",\"events\":" + String(_player->getEventCount());
    resp += ",\"duration\":" + String(_player->getDurationMs());
    resp += ",\"file\":\"" + _player->getFileName() + "\"";
    resp += "}";
    request->send(200, "application/json", resp);

    // Notifier les clients WS
    String wsMsg = "{\"t\":\"midi_loaded\"";
    wsMsg += ",\"file\":\"" + _player->getFileName() + "\"";
    wsMsg += ",\"events\":" + String(_player->getEventCount());
    wsMsg += ",\"duration\":" + String(_player->getDurationMs());
    wsMsg += "}";
    _ws.textAll(wsMsg);
  } else {
    LittleFS.remove(MIDI_FILE_PATH);
    String resp = "{\"ok\":false,\"msg\":\"Format MIDI invalide\"}";
    request->send(400, "application/json", resp);
    _ws.textAll("{\"t\":\"midi_error\",\"msg\":\"Format MIDI invalide\"}");
  }
}

// --- WebSocket ---

void WebConfigurator::onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                                 AwsEventType type, void* arg, uint8_t* data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      if (DEBUG) {
        Serial.print("DEBUG: WS client connecte #");
        Serial.println(client->id());
      }
      break;

    case WS_EVT_DISCONNECT:
      if (DEBUG) {
        Serial.print("DEBUG: WS client deconnecte #");
        Serial.println(client->id());
      }
      break;

    case WS_EVT_DATA: {
      AwsFrameInfo* info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        processWsMessage(client, data, len);
      }
      break;
    }

    default:
      break;
  }
}

void WebConfigurator::processWsMessage(AsyncWebSocketClient* client, uint8_t* data, size_t len) {
  // Parse JSON simple (sans ArduinoJson pour economiser la memoire)
  // Format attendu : {"t":"xxx",...}
  String msg = String((char*)data).substring(0, len);

  if (_instrument == nullptr) return;

  // Extraire le type de message
  int tIdx = msg.indexOf("\"t\":\"");
  if (tIdx < 0) return;
  tIdx += 5;
  int tEnd = msg.indexOf("\"", tIdx);
  if (tEnd < 0) return;
  String type = msg.substring(tIdx, tEnd);

  if (type == "non") {
    // Note On : {"t":"non","n":82,"v":100}
    int nIdx = msg.indexOf("\"n\":");
    int vIdx = msg.indexOf("\"v\":");
    if (nIdx < 0) return;

    uint8_t note = (uint8_t)msg.substring(nIdx + 4).toInt();
    uint8_t vel = _webVelocity;
    if (vIdx >= 0) {
      vel = (uint8_t)msg.substring(vIdx + 4).toInt();
    }
    _instrument->noteOn(note, vel);

  } else if (type == "nof") {
    // Note Off : {"t":"nof","n":82}
    int nIdx = msg.indexOf("\"n\":");
    if (nIdx < 0) return;
    uint8_t note = (uint8_t)msg.substring(nIdx + 4).toInt();
    _instrument->noteOff(note);

  } else if (type == "cc") {
    // Control Change : {"t":"cc","c":7,"v":100}
    int cIdx = msg.indexOf("\"c\":");
    int vIdx = msg.indexOf("\"v\":");
    if (cIdx < 0 || vIdx < 0) return;
    uint8_t ccNum = (uint8_t)msg.substring(cIdx + 4).toInt();
    uint8_t ccVal = (uint8_t)msg.substring(vIdx + 4).toInt();
    _instrument->handleControlChange(ccNum, ccVal);

  } else if (type == "velocity") {
    // Changer velocity par defaut : {"t":"velocity","v":100}
    int vIdx = msg.indexOf("\"v\":");
    if (vIdx >= 0) {
      _webVelocity = (uint8_t)msg.substring(vIdx + 4).toInt();
      if (_webVelocity < 1) _webVelocity = 1;
      if (_webVelocity > 127) _webVelocity = 127;
    }

  } else if (type == "play") {
    if (_player) _player->play();
  } else if (type == "pause") {
    if (_player) _player->pause();
  } else if (type == "stop") {
    if (_player) _player->stop();
  } else if (type == "panic") {
    _instrument->allSoundOff();
  }
}

void WebConfigurator::broadcastStatus() {
  if (_ws.count() == 0) return;

  String json = "{\"t\":\"status\"";

  if (_instrument) {
    NoteSequencer& seq = _instrument->getSequencer();
    json += ",\"playing\":" + String(seq.isPlaying() ? "true" : "false");
    json += ",\"state\":" + String(seq.getState());
    json += ",\"cc7\":" + String(_instrument->getCCVolume());
    json += ",\"cc11\":" + String(_instrument->getCCExpression());
    json += ",\"cc1\":" + String(_instrument->getCCModulation());
    json += ",\"cc2\":" + String(_instrument->getCCBreath());
  }

  if (_player) {
    json += ",\"ps\":" + String(_player->getState());
    if (_player->isFileLoaded()) {
      json += ",\"pp\":" + String(_player->getProgressPercent(), 1);
      json += ",\"ppos\":" + String(_player->getPositionMs());
    }
  }

  json += ",\"heap\":" + String(ESP.getFreeHeap());
  json += "}";

  _ws.textAll(json);
}
