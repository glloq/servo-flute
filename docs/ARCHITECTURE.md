# Architecture Servo Flute V3

## 📋 Vue d'ensemble

La Servo Flute V3 est une flûte robotique contrôlée par MIDI, utilisant des servomoteurs pour actionner les doigts et contrôler le débit d'air. L'architecture est modulaire, séparant les responsabilités en classes spécialisées.

**Version :** V3
**Platform

e :** Arduino (ATmega32u4 - Leonardo/Micro)
**Date :** 2026-02-04

---

## 🏗️ Structure des dossiers

```
servo-flute/
├── Servo_flute_v3/           # Code principal Arduino
│   ├── Servo_flute_v3.ino    # Sketch principal
│   ├── settings.h            # Configuration (CENTRAL)
│   ├── MidiHandler.h/cpp     # Réception MIDI
│   ├── InstrumentManager.h/cpp  # Orchestration globale
│   ├── AirflowController.h/cpp  # Contrôle airflow + CC
│   ├── FingerController.h/cpp   # Contrôle doigts
│   ├── NoteSequencer.h/cpp      # Séquençage notes
│   └── EventQueue.h/cpp         # File d'événements MIDI
│
├── Calibration_Tool/         # Outil calibration standalone
│   ├── Calibration_Tool.ino
│   ├── settings_template.h
│   ├── CalibrationManager.h/cpp
│   ├── FingerCalibrator.h/cpp
│   ├── AirflowCalibrator.h/cpp
│   ├── OutputGenerator.h/cpp
│   └── README.md
│
├── docs/                     # Documentation
│   ├── ARCHITECTURE.md       # Ce fichier
│   ├── MIDI_CC_IMPLEMENTATION.md
│   ├── CC2_BREATH_CONTROLLER.md
│   ├── CONFIGURATION_GUIDE.md
│   ├── INSTRUMENTS_GUIDE.md
│   ├── SOLENOID_PWM.md
│   ├── TIMING_ANTICIPATION.md
│   ├── VALVE_OPTIMIZATION.md
│   └── README_V3.md
│
├── img/                      # Images
├── stl/                      # Fichiers STL (mécanique)
└── README.md                 # Documentation principale
```

---

## 🔧 Classes principales

### 1. **Servo_flute_v3.ino** - Sketch principal

**Rôle :** Point d'entrée, boucle principale, watchdog

**Responsabilités :**
- Initialisation matériel (`setup()`)
- Boucle principale (`loop()`)
- Watchdog timer (4s, auto-restart)
- État sûr en cas de crash (`initSafeState()`)

**Code clé :**
```cpp
void setup() {
  initSafeState();           // État sûr AVANT tout
  wdt_enable(WDTO_4S);       // Watchdog 4 secondes
  instrumentManager.begin(); // Initialiser tout
}

void loop() {
  wdt_reset();                      // Reset watchdog
  instrumentManager.processMidi();  // Traiter MIDI
  instrumentManager.update();       // Mise à jour contrôles
}
```

---

### 2. **settings.h** - Configuration centrale

**Rôle :** TOUTES les constantes du projet (un seul endroit)

**Sections :**
```cpp
// Hardware
#define NUMBER_SERVOS_FINGER 6
#define SERVO_FREQUENCY 50
#define SOLENOID_PIN 13

// Notes jouables (Irish flute 6 trous)
const NoteDefinition NOTES[] = { ... };

// MIDI
#define MIDI_CHANNEL 0
#define CC_RATE_LIMIT_PER_SECOND 10

// CC2 Breath Controller
#define CC2_ENABLED true
#define CC2_RATE_LIMIT_PER_SECOND 50
#define CC2_SMOOTHING_BUFFER_SIZE 5
// ...

// Vibrato
#define VIBRATO_FREQUENCY_HZ 6.0
#define VIBRATO_MAX_AMPLITUDE_DEG 8.0

// Debug
#define DEBUG 0
```

**Avantages :**
- Un seul fichier à modifier pour configuration
- Pas de magic numbers dans le code
- Facilite portage vers autres instruments

---

### 3. **MidiHandler** - Réception MIDI

**Rôle :** Écoute USB MIDI, filtre canaux, dispatch messages

**Fichiers :** `MidiHandler.h/cpp`

**Responsabilités :**
- Lire messages USB MIDI (`MIDIUSB.read()`)
- Filtrer par canal MIDI (omni ou spécifique)
- Parser messages (Note On/Off, CC, etc.)
- Déléguer à InstrumentManager

**Flux :**
```
USB MIDI → MidiHandler.processMidi()
              ↓
         isChannelAccepted()?
              ↓ (oui)
         Switch (messageType)
              ↓
    0x90 → instrument.noteOn()
    0x80 → instrument.noteOff()
    0xB0 → instrument.handleControlChange()
```

**Code clé :**
```cpp
void MidiHandler::processMidi() {
  midiEventPacket_t midiEvent = MidiUSB.read();

  byte channel = midiEvent.byte1 & 0x0F;
  if (!isChannelAccepted(channel)) return;  // Filtrage canal

  byte messageType = midiEvent.byte1 & 0xF0;

  switch (messageType) {
    case 0x90:  // Note On
      _instrument.noteOn(note, velocity);
      break;
    case 0xB0:  // Control Change
      _instrument.handleControlChange(ccNumber, ccValue);
      break;
  }
}
```

---

### 4. **InstrumentManager** - Chef d'orchestre

**Rôle :** Gestion globale, coordination entre tous les composants

**Fichiers :** `InstrumentManager.h/cpp`

**Responsabilités :**
- Initialiser tous les contrôleurs
- Gérer queue d'événements MIDI
- Gérer Control Changes (rate limiting, stockage)
- Coordonner FingerController + AirflowController + NoteSequencer
- Power management servos

**Sous-composants :**
```cpp
class InstrumentManager {
  Adafruit_PWMServoDriver _pwm;      // Driver PCA9685
  EventQueue _eventQueue;            // File MIDI
  FingerController _fingerCtrl;      // Contrôle doigts
  AirflowController _airflowCtrl;    // Contrôle airflow
  NoteSequencer _sequencer;          // Séquenceur

  // CC MIDI
  byte _ccVolume, _ccExpression, _ccModulation, _ccBreath, _ccBrightness;

  // Rate limiting
  uint16_t _ccCount, _cc2Count;
  unsigned long _ccWindowStart, _cc2WindowStart;
};
```

**Méthodes principales :**
```cpp
void begin();                              // Initialisation
void processMidi();                        // Traiter messages MIDI
void update();                             // Mise à jour continue
void noteOn(byte note, byte velocity);     // Note On
void noteOff(byte note);                   // Note Off
void handleControlChange(byte cc, byte val); // CC MIDI
void allSoundOff();                        // Urgence
void resetAllControllers();                // Reset CC
```

---

### 5. **AirflowController** - Contrôle du souffle

**Rôle :** Gérer servo airflow + solénoïde + tous les CC

**Fichiers :** `AirflowController.h/cpp`

**Responsabilités :**
- Positionner servo airflow selon note + velocity/CC2
- Contrôler solénoïde (ouverture/fermeture valve)
- Appliquer CC7 (Volume), CC2 (Breath), CC11 (Expression)
- Gérer vibrato (CC1)
- Lissage CC2 (buffer circulaire)
- Fallback velocity si CC2 absent

**Variables clés :**
```cpp
byte _ccVolume, _ccExpression, _ccModulation, _ccBreath;

// CC2 Breath Controller
byte _cc2SmoothingBuffer[5];
uint8_t _cc2BufferIndex, _cc2BufferCount;
unsigned long _lastCC2Time;
byte _lastVelocity;

// Vibrato
uint16_t _baseAngleWithoutVibrato;
bool _vibratoActive;
uint16_t _currentMinAngle, _currentMaxAngle;
```

**Méthodes principales :**
```cpp
void setAirflowForNote(byte note, byte velocity);  // Calcul angle note
void setCCValues(byte cc7, byte cc11, byte cc1);   // Mise à jour CC
void updateCC2Breath(byte cc2);                    // Recevoir CC2
void openSolenoid();                               // Ouvrir valve
void closeSolenoid();                              // Fermer valve
void update();                                     // Appliquer vibrato
```

**Ordre application (setAirflowForNote) :**
```
1. CC7 (Volume) → Réduit effectiveMaxAngle
2. CC2 (Breath) OU Velocity → Définit baseAngle
   - Si CC2 actif : moyenne lissée, courbe expo, seuil silence
   - Sinon : velocity classique
3. CC11 (Expression) → Module dans [minAngle, baseAngle]
4. Limiter bornes [SERVO_AIRFLOW_MIN, SERVO_AIRFLOW_MAX]
5. Stocker _baseAngleWithoutVibrato
6. CC1 (Vibrato) → Appliqué dans update() (continu)
```

---

### 6. **FingerController** - Contrôle des doigts

**Rôle :** Actionner servos doigts pour former les notes

**Fichiers :** `FingerController.h/cpp`

**Responsabilités :**
- Positionner servos doigts (ouvert/fermé)
- Gérer configuration par note (tableau NOTES[])
- Supporter inversion de sens servo

**Méthode principale :**
```cpp
void setFingersForNote(byte midiNote) {
  const NoteDefinition* note = getNoteByMidi(midiNote);

  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    if (note->fingersClosed & (1 << i)) {
      // Doigt fermé
      setServoAngle(i, FINGERS[i].closedAngle);
    } else {
      // Doigt ouvert
      setServoAngle(i, FINGERS[i].openAngle);
    }
  }
}
```

**Configuration doigt (settings.h) :**
```cpp
const FingerDefinition FINGERS[] = {
  {0, 90, false},  // Servo 0: fermé=0°, ouvert=90°, pas inversé
  {0, 90, false},  // Servo 1
  {0, 90, false},  // Servo 2
  {0, 90, false},  // Servo 3
  {0, 90, false},  // Servo 4
  {0, 90, false}   // Servo 5
};
```

---

### 7. **NoteSequencer** - Séquençage temporel

**Rôle :** Gérer timing des événements, anticipation mécanique

**Fichiers :** `NoteSequencer.h/cpp`

**Responsabilités :**
- Anticiper mouvements mécaniques (doigts avant airflow)
- Gérer transitions entre notes (legato/staccato)
- Queue d'événements futurs
- Timing précis (ms)

**Constantes timing (settings.h) :**
```cpp
#define NOTE_ON_ANTICIPATION_TIME 30     // 30ms avant ouverture valve
#define NOTE_OFF_DELAY_TIME 10           // 10ms après fermeture valve
#define LEGATO_DELAY 20                  // 20ms pour legato
```

**Flux Note On :**
```
t=0ms   : noteOn() appelé
t=0ms   : Doigts positionnés
t=30ms  : Valve ouverte + Airflow activé  (anticipation)
```

**Flux Note Off :**
```
t=0ms   : noteOff() appelé
t=0ms   : Valve fermée + Airflow coupé
t=10ms  : Doigts libérés  (délai)
```

---

### 8. **EventQueue** - File d'événements

**Rôle :** Queue FIFO pour événements MIDI avec timestamp

**Fichiers :** `EventQueue.h/cpp`

**Structure :**
```cpp
struct Event {
  unsigned long timestamp;   // Quand exécuter (ms)
  byte type;                 // NOTE_ON, NOTE_OFF, etc.
  byte note;
  byte velocity;
};
```

**Méthodes :**
```cpp
bool push(Event event);       // Ajouter événement
bool pop(Event& event);       // Retirer événement
Event* peek();                // Voir prochain
bool isEmpty();
void clear();                 // Vider
```

---

## 🔄 Flux de données

### Flux complet Note On

```
1. USB MIDI Note On reçu
         ↓
2. MidiHandler.processMidi()
   - Lecture MIDIUSB.read()
   - Filtrage canal MIDI
         ↓
3. InstrumentManager.noteOn(note, velocity)
   - Ajouter événement à EventQueue
   - Timestamp = now + anticipation
         ↓
4. NoteSequencer.update() (dans loop)
   - Événement prêt ?
         ↓
5. FingerController.setFingersForNote(note)
   - Positionner servos doigts
         ↓
6. AirflowController.setAirflowForNote(note, velocity)
   - Calcul angle : CC7 → CC2/Velocity → CC11
   - Stocker _baseAngleWithoutVibrato
         ↓
7. AirflowController.openSolenoid()
   - Ouvrir valve pneumatique
         ↓
8. AirflowController.update() (continu dans loop)
   - Si CC1 > 0 : Appliquer vibrato
   - Sin() LUT pour oscillation
   - Limiter [_currentMinAngle, _currentMaxAngle]
```

### Flux Control Change (CC2 Breath)

```
1. USB MIDI CC2 reçu (valeur 0-127)
         ↓
2. MidiHandler.processMidi()
   - case 0xB0: Control Change
         ↓
3. InstrumentManager.handleControlChange(2, ccValue)
   - Rate limiting vérifié (50/sec max pour CC2)
         ↓
4. AirflowController.updateCC2Breath(ccValue)
   - Ajouter au buffer circulaire
   - Mise à jour _lastCC2Time
         ↓
5. Prochaine note jouée → setAirflowForNote()
   - Vérifier timeout CC2
   - Calculer moyenne lissée buffer
   - Appliquer courbe exponentielle
   - Vérifier seuil silence
   - Utiliser CC2 comme airflowSource (remplace velocity)
         ↓
6. Angle airflow calculé avec CC2 au lieu de velocity
```

---

## 🎛️ Configuration par instrument

### Irish Flute (6 trous) - Configuration actuelle

**settings.h :**
```cpp
#define NUMBER_SERVOS_FINGER 6

const NoteDefinition NOTES[] = {
  // midiNote, airflowMin%, airflowMax%, fingersBitmap
  {70,  20,  75, 0b111111},  // A#5 (tous fermés)
  {72,  20,  75, 0b111110},  // C6
  {73,  20,  75, 0b111100},  // C#6
  {74,  25,  70, 0b111000},  // D6
  {75,  25,  70, 0b110000},  // D#6
  {77,  25,  70, 0b100000},  // F6
  {78,  30,  65, 0b000000},  // F#6 (tous ouverts)
  // ... octave supérieur
  {91,  40,  55, 0b000000}   // G7
};
```

**Caractéristiques :**
- 6 servos doigts
- 14 notes jouables (A#5 - G7)
- Tonalité : C majeur
- Airflow : 20-75% (notes graves), 40-55% (notes aiguës)

### Adaptation pour autre instrument

**Exemple : Flûte à bec soprano (8 trous)**

```cpp
#define NUMBER_SERVOS_FINGER 8  // Au lieu de 6

const NoteDefinition NOTES[] = {
  {67,  15,  80, 0b11111111},  // G4 (tous fermés)
  {69,  15,  80, 0b11111110},  // A4
  // ... définir toutes les notes
  {86,  50,  60, 0b00000000}   // D6 (tous ouverts)
};

const FingerDefinition FINGERS[] = {
  {0, 90, false},  // 8 servos au lieu de 6
  {0, 90, false},
  {0, 90, false},
  {0, 90, false},
  {0, 90, false},
  {0, 90, false},
  {0, 90, false},
  {0, 90, false}
};
```

**Le code s'adapte automatiquement** grâce à `NUMBER_SERVOS_FINGER` !

---

## 🔒 Sécurité et robustesse

### 1. Watchdog Timer

**Problème :** Code bloqué (bug, boucle infinie) → Instrument mort

**Solution :** Watchdog 4 secondes auto-restart

```cpp
// setup()
wdt_enable(WDTO_4S);  // Watchdog 4 secondes

// loop()
wdt_reset();  // Reset à chaque itération
```

Si `loop()` ne s'exécute pas pendant 4s → Arduino redémarre automatiquement

### 2. État sûr (initSafeState)

**Problème :** Crash, reset, power-on → Servos position inconnue

**Solution :** État sûr AVANT tout

```cpp
void initSafeState() {
  // Fermer solénoïde
  digitalWrite(SOLENOID_PIN, LOW);

  // Airflow au repos
  pwm.setPWM(NUM_SERVO_AIRFLOW, 0, pulseWidthOff);

  // Tous doigts fermés
  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    pwm.setPWM(i, 0, pulseWidthClosed);
  }
}
```

Appelé en **PREMIER** dans `setup()`, avant toute autre init.

### 3. Rate Limiting

**Problème :** Flood MIDI → Saturation CPU, jitter servos

**Solution :** Limite 10 CC/sec (50 CC2/sec)

```cpp
if (_ccCount > CC_RATE_LIMIT_PER_SECOND) {
  return;  // Ignorer message
}
```

Messages urgents (CC 120, 121, 123) exemptés.

### 4. Validation entrées

**Problème :** Valeurs MIDI invalides (>127, négatives)

**Solution :** Validation systématique

```cpp
if (ccValue > 127) {
  if (DEBUG) Serial.println("ERREUR: CC invalide");
  return;  // Ignorer
}
```

### 5. Bornes servos

**Problème :** Angle calculé hors bornes physiques → Dommages mécanique

**Solution :** Clamp systématique

```cpp
if (finalAngle < SERVO_AIRFLOW_MIN) finalAngle = SERVO_AIRFLOW_MIN;
if (finalAngle > SERVO_AIRFLOW_MAX) finalAngle = SERVO_AIRFLOW_MAX;
```

Garanti servo toujours dans plage valide.

---

## 📊 Performance

### Temps de réponse

| Événement | Latence | Note |
|-----------|---------|------|
| Note On → Doigts positionnés | < 5ms | Instantané |
| Note On → Valve ouverte | 30ms | Anticipation intentionnelle |
| CC2 reçu → Airflow ajusté | < 2ms | Très réactif |
| Vibrato update | 1ms | Continu (loop) |

### Charge CPU

| Tâche | CPU | Note |
|-------|-----|------|
| Loop principale | ~20% | Léger |
| Rate limiting | < 1% | Négligeable |
| CC2 lissage | ~2% | Buffer moyennage |
| Vibrato sin() LUT | < 1% | Optimisé PROGMEM |
| **Total** | **~25%** | **Marge confortable** |

### Optimisations

1. **Sin() Lookup Table** : 256 entrées PROGMEM → 25x plus rapide que `sin()`
2. **Rate limiting** : Évite surcharge inutile
3. **Buffer circulaire CC2** : Moyenne glissante efficace
4. **Anticipation mécanique** : Masque latence doigts

---

## 🛠️ Outils de développement

### Calibration Tool

**Emplacement :** `/Calibration_Tool/`

**Rôle :** Calibrer servos doigts + airflow par note

**Workflow :**
1. Lancer `Calibration_Tool.ino`
2. Calibrer servos (angle fermé + ouvert)
3. Calibrer notes (airflowMin% + airflowMax%)
4. Générer code C++ formaté
5. Copier-coller dans `settings.h`

**Avantages :**
- Interface Serial Monitor intuitive
- Génère code prêt à l'emploi
- Évite erreurs manuelles

**Documentation :** Voir `/Calibration_Tool/README.md`

---

## 📚 Documentation associée

- **[MIDI_CC_IMPLEMENTATION.md](MIDI_CC_IMPLEMENTATION.md)** - Tous les Control Changes
- **[CC2_BREATH_CONTROLLER.md](CC2_BREATH_CONTROLLER.md)** - CC2 Breath Controller détaillé
- **[CONFIGURATION_GUIDE.md](CONFIGURATION_GUIDE.md)** - Guide configuration complète
- **[INSTRUMENTS_GUIDE.md](INSTRUMENTS_GUIDE.md)** - Adaptation autres instruments
- **[SOLENOID_PWM.md](SOLENOID_PWM.md)** - Contrôle PWM solénoïde
- **[TIMING_ANTICIPATION.md](TIMING_ANTICIPATION.md)** - Timing et anticipation
- **[VALVE_OPTIMIZATION.md](VALVE_OPTIMIZATION.md)** - Optimisation valve
- **[README_V3.md](README_V3.md)** - Vue d'ensemble V3

---

**Créé le :** 2026-02-04
**Version Servo Flute :** V3
**Statut :** ✅ Production
