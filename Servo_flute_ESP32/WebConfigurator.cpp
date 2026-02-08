#include "WebConfigurator.h"
#include "InstrumentManager.h"
#include "WirelessManager.h"
#include "web_content.h"
#include <WiFi.h>

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

  // API Config POST (body handler pour JSON)
  _server.on("/api/config", HTTP_POST,
    [this](AsyncWebServerRequest* request) {
      // Reponse envoyee dans le body handler a la fin
      // Si on arrive ici sans body, renvoyer erreur
      if (_configBody.length() == 0) {
        request->send(400, "application/json", "{\"ok\":false,\"msg\":\"Body vide\"}");
      }
    },
    NULL,  // pas d'upload handler
    [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
      handleApiConfigPost(request, data, len, index, total);
    }
  );

  // API Config Reset
  _server.on("/api/config/reset", HTTP_POST, [this](AsyncWebServerRequest* request) {
    handleApiConfigReset(request);
  });

  // API WiFi Scan (lance le scan)
  _server.on("/api/wifi/scan", HTTP_GET, [this](AsyncWebServerRequest* request) {
    if (_wirelessManager) {
      _wirelessManager->getWifiMidi().startWifiScan();
      request->send(200, "application/json", "{\"ok\":true,\"msg\":\"Scan lance\"}");
    } else {
      request->send(500, "application/json", "{\"ok\":false}");
    }
  });

  // API WiFi Scan Results
  _server.on("/api/wifi/results", HTTP_GET, [this](AsyncWebServerRequest* request) {
    if (_wirelessManager) {
      bool done = _wirelessManager->getWifiMidi().isScanComplete();
      String json = "{\"done\":" + String(done ? "true" : "false");
      if (done) {
        json += ",\"networks\":" + _wirelessManager->getWifiMidi().getScanResultsJson();
      }
      json += "}";
      request->send(200, "application/json", json);
    } else {
      request->send(500, "application/json", "{\"ok\":false}");
    }
  });

  // API WiFi Connect (POST JSON {"ssid":"...","pass":"..."})
  _server.on("/api/wifi/connect", HTTP_POST,
    [this](AsyncWebServerRequest* request) {
      if (_configBody.length() == 0) {
        request->send(400, "application/json", "{\"ok\":false,\"msg\":\"Body vide\"}");
      }
    },
    NULL,
    [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
      if (index == 0) { _configBody = ""; _configBody.reserve(total); }
      _configBody += String((char*)data).substring(0, len);
      if (index + len == total) {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, _configBody);
        if (err || !doc.containsKey("ssid")) {
          request->send(400, "application/json", "{\"ok\":false,\"msg\":\"JSON invalide\"}");
        } else if (_wirelessManager) {
          const char* ssid = doc["ssid"];
          const char* pass = doc["pass"] | "";
          request->send(200, "application/json", "{\"ok\":true,\"msg\":\"Connexion en cours...\"}");
          _wirelessManager->getWifiMidi().connectToNetwork(ssid, pass);
        } else {
          request->send(500, "application/json", "{\"ok\":false}");
        }
        _configBody = "";
      }
    }
  );

  // API WiFi Status
  _server.on("/api/wifi/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
    String json = "{";
    if (_wirelessManager) {
      WifiMidiHandler& wm = _wirelessManager->getWifiMidi();
      json += "\"state\":" + String(wm.getState());
      json += ",\"ip\":\"" + wm.getIPAddress() + "\"";
      json += ",\"ap\":" + String(wm.isAPMode() ? "true" : "false");
      json += ",\"ssid\":\"" + String(cfg.wifiSsid) + "\"";
      if (wm.getState() == WIFI_STATE_STA_CONNECTED) {
        json += ",\"rssi\":" + String(WiFi.RSSI());
      }
    }
    json += "}";
    request->send(200, "application/json", json);
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
  // Lire depuis cfg (RuntimeConfig) au lieu des #defines
  String json = "{";
  json += "\"midi_ch\":" + String(cfg.midiChannel);
  json += ",\"servo_delay\":" + String(cfg.servoToSolenoidDelayMs);
  json += ",\"valve_interval\":" + String(cfg.minNoteIntervalForValveCloseMs);
  json += ",\"min_note_dur\":" + String(cfg.minNoteDurationMs);
  json += ",\"air_off\":" + String(cfg.servoAirflowOff);
  json += ",\"air_min\":" + String(cfg.servoAirflowMin);
  json += ",\"air_max\":" + String(cfg.servoAirflowMax);
  json += ",\"vib_freq\":" + String(cfg.vibratoFrequencyHz, 1);
  json += ",\"vib_amp\":" + String(cfg.vibratoMaxAmplitudeDeg, 1);
  json += ",\"cc_vol\":" + String(cfg.ccVolumeDefault);
  json += ",\"cc_expr\":" + String(cfg.ccExpressionDefault);
  json += ",\"cc_mod\":" + String(cfg.ccModulationDefault);
  json += ",\"cc_breath\":" + String(cfg.ccBreathDefault);
  json += ",\"cc_bright\":" + String(cfg.ccBrightnessDefault);
  json += ",\"cc2_on\":" + String(cfg.cc2Enabled ? "true" : "false");
  json += ",\"cc2_thr\":" + String(cfg.cc2SilenceThreshold);
  json += ",\"cc2_curve\":" + String(cfg.cc2ResponseCurve, 2);
  json += ",\"cc2_timeout\":" + String(cfg.cc2TimeoutMs);
  json += ",\"sol_act\":" + String(cfg.solenoidPwmActivation);
  json += ",\"sol_hold\":" + String(cfg.solenoidPwmHolding);
  json += ",\"sol_time\":" + String(cfg.solenoidActivationTimeMs);
  json += ",\"angle_open\":" + String(cfg.fingerAngleOpen);
  json += ",\"device\":\"" + String(cfg.deviceName) + "\"";
  json += ",\"wifi_ssid\":\"" + String(cfg.wifiSsid) + "\"";
  json += ",\"time_unpower\":" + String(cfg.timeUnpower);
  json += ",\"num_notes\":" + String(NUMBER_NOTES);
  json += ",\"num_fingers\":" + String(NUMBER_SERVOS_FINGER);

  // Doigts
  json += ",\"fingers\":[";
  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    if (i > 0) json += ",";
    json += "{\"ch\":" + String(FINGERS[i].pcaChannel);
    json += ",\"a\":" + String(cfg.fingerClosedAngle[i]);
    json += ",\"d\":" + String(cfg.fingerDirection[i]) + "}";
  }
  json += "]";

  // Notes jouables avec airflow configurable
  json += ",\"notes\":[";
  for (int i = 0; i < NUMBER_NOTES; i++) {
    if (i > 0) json += ",";
    json += "{\"midi\":" + String(NOTES[i].midiNote);
    json += ",\"air_min\":" + String(cfg.noteAirflowMin[i]);
    json += ",\"air_max\":" + String(cfg.noteAirflowMax[i]);
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

void WebConfigurator::handleApiConfigPost(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
  // Accumuler le body
  if (index == 0) {
    _configBody = "";
    _configBody.reserve(total);
  }
  _configBody += String((char*)data).substring(0, len);

  // Fin du body
  if (index + len == total) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, _configBody);

    if (err) {
      request->send(400, "application/json", "{\"ok\":false,\"msg\":\"JSON invalide\"}");
      _configBody = "";
      return;
    }

    // Appliquer les valeurs (chaque champ est optionnel)
    if (doc.containsKey("midi_ch")) cfg.midiChannel = doc["midi_ch"];
    if (doc.containsKey("servo_delay")) cfg.servoToSolenoidDelayMs = doc["servo_delay"];
    if (doc.containsKey("valve_interval")) cfg.minNoteIntervalForValveCloseMs = doc["valve_interval"];
    if (doc.containsKey("min_note_dur")) cfg.minNoteDurationMs = doc["min_note_dur"];
    if (doc.containsKey("air_off")) cfg.servoAirflowOff = doc["air_off"];
    if (doc.containsKey("air_min")) cfg.servoAirflowMin = doc["air_min"];
    if (doc.containsKey("air_max")) cfg.servoAirflowMax = doc["air_max"];
    if (doc.containsKey("vib_freq")) cfg.vibratoFrequencyHz = doc["vib_freq"];
    if (doc.containsKey("vib_amp")) cfg.vibratoMaxAmplitudeDeg = doc["vib_amp"];
    if (doc.containsKey("cc_vol")) cfg.ccVolumeDefault = doc["cc_vol"];
    if (doc.containsKey("cc_expr")) cfg.ccExpressionDefault = doc["cc_expr"];
    if (doc.containsKey("cc_mod")) cfg.ccModulationDefault = doc["cc_mod"];
    if (doc.containsKey("cc_breath")) cfg.ccBreathDefault = doc["cc_breath"];
    if (doc.containsKey("cc_bright")) cfg.ccBrightnessDefault = doc["cc_bright"];
    if (doc.containsKey("cc2_on")) cfg.cc2Enabled = doc["cc2_on"].as<bool>();
    if (doc.containsKey("cc2_thr")) cfg.cc2SilenceThreshold = doc["cc2_thr"];
    if (doc.containsKey("cc2_curve")) cfg.cc2ResponseCurve = doc["cc2_curve"];
    if (doc.containsKey("cc2_timeout")) cfg.cc2TimeoutMs = doc["cc2_timeout"];
    if (doc.containsKey("sol_act")) cfg.solenoidPwmActivation = doc["sol_act"];
    if (doc.containsKey("sol_hold")) cfg.solenoidPwmHolding = doc["sol_hold"];
    if (doc.containsKey("sol_time")) cfg.solenoidActivationTimeMs = doc["sol_time"];
    if (doc.containsKey("angle_open")) cfg.fingerAngleOpen = doc["angle_open"];
    if (doc.containsKey("time_unpower")) cfg.timeUnpower = doc["time_unpower"];

    if (doc.containsKey("device")) {
      strncpy(cfg.deviceName, doc["device"] | cfg.deviceName, sizeof(cfg.deviceName) - 1);
      cfg.deviceName[sizeof(cfg.deviceName) - 1] = '\0';
    }
    if (doc.containsKey("wifi_ssid")) {
      strncpy(cfg.wifiSsid, doc["wifi_ssid"] | "", sizeof(cfg.wifiSsid) - 1);
      cfg.wifiSsid[sizeof(cfg.wifiSsid) - 1] = '\0';
    }
    if (doc.containsKey("wifi_pass")) {
      strncpy(cfg.wifiPassword, doc["wifi_pass"] | "", sizeof(cfg.wifiPassword) - 1);
      cfg.wifiPassword[sizeof(cfg.wifiPassword) - 1] = '\0';
    }

    // Doigts
    if (doc.containsKey("fingers")) {
      JsonArray fingers = doc["fingers"];
      for (int i = 0; i < NUMBER_SERVOS_FINGER && i < (int)fingers.size(); i++) {
        if (fingers[i].containsKey("a")) cfg.fingerClosedAngle[i] = fingers[i]["a"];
        if (fingers[i].containsKey("d")) cfg.fingerDirection[i] = fingers[i]["d"];
      }
    }

    // Notes airflow
    if (doc.containsKey("notes_air")) {
      JsonArray notes = doc["notes_air"];
      for (int i = 0; i < NUMBER_NOTES && i < (int)notes.size(); i++) {
        if (notes[i].containsKey("mn")) cfg.noteAirflowMin[i] = notes[i]["mn"];
        if (notes[i].containsKey("mx")) cfg.noteAirflowMax[i] = notes[i]["mx"];
      }
    }

    // Sauvegarder sur LittleFS
    bool saved = ConfigStorage::save();

    if (DEBUG) {
      Serial.println("DEBUG: WebConfigurator - Config mise a jour via web");
    }

    String resp = "{\"ok\":" + String(saved ? "true" : "false") + "}";
    request->send(200, "application/json", resp);
    _configBody = "";
  }
}

void WebConfigurator::handleApiConfigReset(AsyncWebServerRequest* request) {
  ConfigStorage::resetToDefaults();

  if (DEBUG) {
    Serial.println("DEBUG: WebConfigurator - Config reset aux defauts");
  }

  request->send(200, "application/json", "{\"ok\":true}");
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

  } else if (type == "test_finger") {
    // {"t":"test_finger","i":0,"a":90}
    int iIdx = msg.indexOf("\"i\":");
    int aIdx = msg.indexOf("\"a\":");
    if (iIdx >= 0 && aIdx >= 0) {
      int fi = msg.substring(iIdx + 4).toInt();
      int angle = msg.substring(aIdx + 4).toInt();
      _instrument->getFingerCtrl().testFingerAngle(fi, (uint16_t)angle);
    }

  } else if (type == "test_air") {
    // {"t":"test_air","a":60}
    int aIdx = msg.indexOf("\"a\":");
    if (aIdx >= 0) {
      int angle = msg.substring(aIdx + 4).toInt();
      _instrument->getAirflowCtrl().testAirflowAngle((uint16_t)angle);
    }

  } else if (type == "test_sol") {
    // {"t":"test_sol","o":1}
    int oIdx = msg.indexOf("\"o\":");
    if (oIdx >= 0) {
      int val = msg.substring(oIdx + 4).toInt();
      _instrument->getAirflowCtrl().testSolenoid(val != 0);
    }

  } else if (type == "test_note") {
    // {"t":"test_note","n":84} - joue le pattern de doigts + airflow pour une note
    int nIdx = msg.indexOf("\"n\":");
    if (nIdx >= 0) {
      uint8_t note = (uint8_t)msg.substring(nIdx + 4).toInt();
      _instrument->getFingerCtrl().setFingerPatternForNote(note);
      _instrument->getAirflowCtrl().setAirflowForNote(note, _webVelocity);
    }
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
