# Architecture ESP32

## Diagramme des modules

```
                        Servo_flute_ESP32.ino
                               |
                    +----------+----------+
                    |                     |
             InstrumentManager      WirelessManager
              |     |     |          |    |    |    |
   FingerCtrl | AirflowCtrl |    BleMidi | WifiMidi |
              |             |           |          |
         NoteSequencer  EventQueue   WebConfigurator
                                     |         |
                              MidiFilePlayer  ConfigStorage
```

## Flux de donnees

### Reception MIDI
```
BLE-MIDI ou rtpMIDI
  -> BleMidiHandler / WifiMidiHandler
    -> isChannelAccepted(channel)
    -> InstrumentManager.noteOn(note, velocity)
      -> NoteSequencer.playNote(note, velocity)
        -> FingerController.setFingerPatternForNote(note)
        -> EventQueue.schedule(solenoid open, delay)
        -> AirflowController.setAirflowForNote(note, velocity)
```

### Clavier virtuel web
```
Navigateur (touch/clic/clavier)
  -> WebSocket {"t":"non","n":82,"v":100}
    -> WebConfigurator.processWsMessage()
      -> InstrumentManager.noteOn(82, 100)
        -> (meme chaine que MIDI)
```

### Configuration
```
Page web Config -> POST /api/config (JSON partiel)
  -> WebConfigurator.handleApiConfigPost()
    -> Mise a jour RuntimeConfig cfg
    -> ConfigStorage::save() -> /config.json (LittleFS)

Au demarrage:
  LittleFS.begin()
  -> ConfigStorage::load() -> lit /config.json -> surcharge cfg
```

## Machine d'etat NoteSequencer

```
IDLE --> POSITIONING --> PLAYING --> STOPPING --> IDLE
  |         |              |            |
  |   setFingerPattern  openSolenoid  closeSolenoid
  |   setAirflow        (via delay)   setAirflowToRest
  |
  +-- noteOn() declenche la transition
```

- **IDLE** : aucune note en cours
- **POSITIONING** : servos en mouvement, attente `servoToSolenoidDelayMs`
- **PLAYING** : solenoide ouvert, note produite
- **STOPPING** : solenoide ferme, retour au repos

## Gestion de l'alimentation

InstrumentManager surveille `_lastActivityTime` :
- Apres `cfg.timeUnpower` ms sans activite -> `powerOffServos()` (PCA9685 OE HIGH)
- A la prochaine note -> `powerOnServos()` (PCA9685 OE LOW)

## WebSocket broadcast

Toutes les 500ms (`WS_STATUS_INTERVAL_MS`), WebConfigurator envoie a tous les clients :
```json
{
  "t": "status",
  "playing": true,
  "state": 2,
  "cc7": 127, "cc11": 127, "cc1": 0, "cc2": 127,
  "ps": 1, "pp": 45.2, "ppos": 12500,
  "heap": 185000
}
```

## Watchdog ESP32

Task WDT configure a 4000ms. `esp_task_wdt_reset()` appele a chaque iteration de `loop()`.
Si le systeme se bloque (I2C freeze, boucle infinie), l'ESP32 redemarrera automatiquement.
Le `initSafeState()` est appele avant tout dans `setup()` pour que le materiel soit dans un etat sur meme apres un crash.

## Memoire

Budget typique (ESP32-WROOM 520KB SRAM) :
- PROGMEM (web_content.h) : ~36KB en flash, 0 en RAM
- RuntimeConfig cfg : ~300 bytes
- ArduinoJson (parsing) : ~2KB temporaire
- NimBLE stack : ~20KB
- WiFi stack : ~40KB
- AsyncWebServer + WS : ~15KB
- Heap libre typique : ~150-180KB
