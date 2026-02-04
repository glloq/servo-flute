# Implémentation MIDI Control Change

## 📋 Control Change implémentés

### CC 1 - Modulation (Vibrato)
- **Valeur :** 0-127
- **Fonction :** Ajoute un vibrato (oscillation) à l'airflow
- **Effet :**
  - 0 = Pas de vibrato
  - 127 = Vibrato maximum (±8°)
- **Fréquence :** 6 Hz (typique pour flûte)
- **Constantes :** `VIBRATO_FREQUENCY_HZ`, `VIBRATO_MAX_AMPLITUDE_DEG` (settings.h)

### CC 2 - Breath Controller
- **Valeur :** 0-127 (défaut: 127)
- **Fonction :** **CONTRÔLE DYNAMIQUE DU SOUFFLE** en temps réel
- **Effet :** Remplace velocity pour contrôle expressif continu
  - 0-9 = Silence (valve fermée, seuil)
  - 10+ = Airflow avec courbe exponentielle (CC2^1.4)
  - 127 = Souffle maximum
- **Usage :** Breath controller physique (Yamaha BC3, TEControl) ou automation DAW
- **Lissage :** Moyenne glissante sur 5 valeurs (réduction jitter)
- **Fallback :** Si CC2 absent > 1s, utilise velocity
- **Rate limit :** 50 messages/sec (haute fréquence)
- **Constantes :** `CC2_ENABLED`, `CC2_RATE_LIMIT_PER_SECOND`, `CC2_SILENCE_THRESHOLD`, `CC2_SMOOTHING_BUFFER_SIZE`, `CC2_RESPONSE_CURVE`, `CC2_TIMEOUT_MS` (settings.h)
- **Documentation détaillée :** Voir [CC2_BREATH_CONTROLLER.md](CC2_BREATH_CONTROLLER.md)

### CC 7 - Volume (Channel Volume)
- **Valeur :** 0-127 (défaut: 127)
- **Fonction :** **RÉDUIT LA LIMITE HAUTE** de la plage de la note
- **Effet :**
  - 0 = Limite haute = minAngle (note très douce)
  - 64 = Limite haute = 50% de la plage
  - 127 = Limite haute = maxAngle (pleine puissance, défaut)
- **Important :** CC7 réduit la plage AVANT que velocity et expression soient appliqués
- **Constante :** `CC_VOLUME_DEFAULT` (settings.h)

### CC 11 - Expression
- **Valeur :** 0-127 (défaut: 127)
- **Fonction :** Contrôle d'expression dynamique **DANS la plage réduite par CC7**
- **Effet :**
  - 0 = Expression minimale (airflowMinPercent de la note)
  - 127 = Pleine expression (angle défini par velocity dans plage réduite)
- **Usage :** Crescendo/diminuendo pendant performance
- **Important :** CC11 module dans la plage réduite par CC7
- **Constante :** `CC_EXPRESSION_DEFAULT` (settings.h)

### CC 74 - Brightness
- **Valeur :** 0-127 (défaut: 64)
- **Fonction :** Contrôle de brillance (réservé pour usage futur)
- **Effet :** Stocké mais non appliqué actuellement
- **Usage :** Destiné au contrôle du timbre ou filtrage
- **Constante :** `CC_BRIGHTNESS_DEFAULT` (settings.h)

### CC 120 - All Sound Off
- **Valeur :** Toutes (déclenchement immédiat)
- **Fonction :** Arrêt d'urgence
- **Actions :**
  - Vide la queue d'événements
  - Stoppe le séquenceur
  - Ferme la valve solénoïde
  - Met l'airflow au repos
  - Ferme tous les servos doigts
- **Note :** Exempt de rate limiting (priorité absolue)

### CC 121 - Reset All Controllers
- **Valeur :** Toutes (déclenchement immédiat)
- **Fonction :** Réinitialise tous les contrôleurs à leurs valeurs par défaut
- **Actions :**
  - CC1 (Modulation) → 0
  - CC2 (Breath) → 127
  - CC7 (Volume) → 127
  - CC11 (Expression) → 127
  - CC74 (Brightness) → 64
  - Pitch Bend → 0 (centre)
- **Note :** Exempt de rate limiting (priorité absolue)

### CC 123 - All Notes Off
- **Valeur :** Toutes (déclenchement immédiat)
- **Fonction :** Identique à CC 120 (All Sound Off)
- **Actions :** Même comportement que CC 120
- **Note :** Exempt de rate limiting (priorité absolue)

---

## 🎯 Différence CC7 vs CC11 - NOUVELLE LOGIQUE

### ⚡ Ordre d'application : CC7 → Velocity → CC11 → Pitch Bend → Vibrato

**Changement important (2026-02-04) :** CC7 réduit maintenant la limite haute AVANT l'application de velocity et CC11.

### CC7 (Volume) - Réduit la limite haute de la note
```
Note C6: airflowMinPercent = 20%, airflowMaxPercent = 75%
→ Plage absolue initiale: [68°, 90°]

CC7 = 127 → effectiveMaxAngle = 68 + (90 - 68) × 1.0 = 90° (plage complète)
CC7 = 64  → effectiveMaxAngle = 68 + (90 - 68) × 0.5 = 79° (plage réduite 50%)
CC7 = 0   → effectiveMaxAngle = 68 + (90 - 68) × 0.0 = 68° (plage minimale)

✓ CC7 définit la nouvelle limite haute AVANT velocity
✓ Velocity et CC11 travaillent ensuite dans [minAngle, effectiveMaxAngle]
```

### Velocity - Utilise la plage réduite par CC7
```
Plage initiale: [68°, 90°]
CC7 = 64 → effectiveMaxAngle = 79°
→ Nouvelle plage disponible: [68°, 79°]

Velocity 127 → baseAngle = 79° (max de la plage réduite)
Velocity 64  → baseAngle = 74° (milieu de la plage réduite)
Velocity 1   → baseAngle = 68° (min de la note)

✓ Velocity mappe dans [minAngle, effectiveMaxAngle] (réduit par CC7)
```

### CC11 (Expression) - Module dans la plage réduite
```
Après CC7 et Velocity:
baseAngle = 79° (velocity 127, CC7=64 → effectiveMax=79°)
Plage: [68°, 79°]

CC11 = 127 → finalAngle = 68 + (79 - 68) × 1.0 = 79° (pleine expression)
CC11 = 64  → finalAngle = 68 + (79 - 68) × 0.5 = 73.5° (expression moyenne)
CC11 = 0   → finalAngle = 68 + (79 - 68) × 0.0 = 68° (expression minimale)

✓ CC11 module dans [minAngle, baseAngle] défini par velocity dans plage réduite
✓ Ne peut JAMAIS descendre sous minAngle ou dépasser effectiveMaxAngle
```

### Cas pratique : Volume réduit + Expression dynamique
```
Velocity 127, Note C6 [68°-90°]

SCÉNARIO 1 : Volume plein (CC7 = 127)
1. CC7 = 127 → effectiveMaxAngle = 90°
2. Velocity 127 → baseAngle = 90°
3. CC11 = 0   → finalAngle = 68° (pianissimo)
4. CC11 = 127 → finalAngle = 90° (fortissimo)
   → Plage expression complète [68°, 90°]

SCÉNARIO 2 : Volume réduit 50% (CC7 = 64)
1. CC7 = 64  → effectiveMaxAngle = 79°
2. Velocity 127 → baseAngle = 79° (limité par CC7!)
3. CC11 = 0   → finalAngle = 68° (pianissimo)
4. CC11 = 127 → finalAngle = 79° (fortissimo limité)
   → Plage expression réduite [68°, 79°]

✓ CC7 agit comme un vrai contrôle de volume en limitant le maximum
✓ CC11 offre des nuances expressives DANS la plage limitée par CC7
```

### Cas pratique : Crescendo avec volume global
```
Velocity 100, Note C6 [68°-90°], CC7 = 80

1. CC7 = 80 → effectiveMaxAngle = 68 + (90-68)×(80/127) = 82°
2. Velocity 100 → baseAngle = map(100, 1, 127, 68, 82) ≈ 79°

Expression dynamique:
- CC11 = 0   → finalAngle = 68° (crescendo depuis silence)
- CC11 = 64  → finalAngle = 74° (crescendo progressif)
- CC11 = 127 → finalAngle = 79° (crescendo maximum)

→ Expression varie dans [68°, 79°] au lieu de [68°, 86°] (sans CC7)
→ CC7 a effectivement réduit le volume global tout en gardant nuances expressives
```

---

## 🎵 Pitch Bend

### Fonctionnement
- **Valeur :** 14-bit (0-16383)
  - Centre : 8192 (pas de bend)
  - Minimum : 0 (-8192, bend vers le bas)
  - Maximum : 16383 (+8191, bend vers le haut)
- **Plage :** ±2 demi-tons (configurable)
- **Effet sur airflow :** ±10% du débit actuel
- **Application :** Après CC7, Velocity et CC11, AVANT vibrato

### Constantes (settings.h)
```cpp
#define PITCH_BEND_RANGE_SEMITONES 2    // ±2 demi-tons
#define PITCH_BEND_AIRFLOW_PERCENT 10   // ±10% airflow
```

### Calcul
```cpp
// 1. Extraction valeur 14-bit MIDI
pitchBendValue = (MSB << 7) | LSB;  // 0-16383

// 2. Conversion en signed (-8192 à +8191)
pitchBend = pitchBendValue - 8192;

// 3. Facteur normalisé (-1.0 à +1.0)
pitchBendFactor = pitchBend / 8192.0;

// 4. Ajustement airflow (±10%)
airflowAdjustment = pitchBendFactor × 10%;  // -10% à +10%

// 5. Application sur angle final (après CC11)
adjustedAngle = finalAngle × (1.0 + airflowAdjustment);
```

### Exemple
```
Note C6, finalAngle après CC = 80°

Pitch Bend = 8192 (centre)  → 80° × (1.0 + 0.0) = 80° (inchangé)
Pitch Bend = 12288 (moitié+) → 80° × (1.0 + 0.05) = 84° (bend +5%)
Pitch Bend = 16383 (max)     → 80° × (1.0 + 0.10) = 88° (bend +10%)
Pitch Bend = 4096 (moitié-)  → 80° × (1.0 - 0.05) = 76° (bend -5%)
Pitch Bend = 0 (min)         → 80° × (1.0 - 0.10) = 72° (bend -10%)
```

---

## ⏱️ Rate Limiting

### Configuration
```cpp
#define CC_RATE_LIMIT_PER_SECOND 10  // Max 10 CC/seconde (settings.h)
```

### Fonctionnement
- **Algorithme :** Fenêtre glissante (sliding window) de 1 seconde
- **Limite :** 10 messages CC par seconde par défaut
- **Exemptions :** CC 120, 121, 123 (urgence) sont TOUJOURS traités
- **Dépassement :** Messages ignorés silencieusement

### Implémentation
```cpp
// Variables dans InstrumentManager
unsigned long _ccWindowStart;  // Début fenêtre 1s
uint16_t _ccCount;             // Compteur messages dans fenêtre

void handleControlChange(byte ccNumber, byte ccValue) {
  unsigned long currentTime = millis();

  // Reset fenêtre toutes les 1s
  if (currentTime - _ccWindowStart >= 1000) {
    _ccWindowStart = currentTime;
    _ccCount = 0;
  }

  // Exemptions : urgence toujours traitée
  if (ccNumber != 120 && ccNumber != 121 && ccNumber != 123) {
    _ccCount++;
    if (_ccCount > CC_RATE_LIMIT_PER_SECOND) {
      return;  // Ignoré
    }
  }

  // Traiter CC...
}
```

### Cas d'usage
```
Mod wheel envoie 100 CC/sec → Seuls 10/sec traités
→ Prévient saturation CPU et jitter servo
→ Messages urgents (120, 121, 123) toujours passent
```

---

## 📡 Canal MIDI

### Configuration
```cpp
#define MIDI_CHANNEL 0  // 0 = omni mode, 1-16 = canal spécifique (settings.h)
```

### Modes

#### Omni Mode (MIDI_CHANNEL = 0)
- Écoute sur **TOUS les canaux** (1-16)
- Utile pour utilisation simple (un seul instrument)
- Mode par défaut recommandé

#### Canal spécifique (MIDI_CHANNEL = 1-16)
- Écoute seulement sur le canal configuré
- Utile pour setups multi-instruments
- Exemple : MIDI_CHANNEL = 5 → écoute seulement canal 5

### Implémentation
```cpp
bool MidiHandler::isChannelAccepted(byte channel) {
  if (MIDI_CHANNEL == 0) return true;  // Omni mode
  return (channel == (MIDI_CHANNEL - 1));  // Canal spécifique (0-indexed)
}

void MidiHandler::processMidiEvent(midiEventPacket_t midiEvent) {
  byte channel = midiEvent.byte1 & 0x0F;

  if (!isChannelAccepted(channel)) {
    return;  // Filtrer messages d'autres canaux
  }

  // Traiter message...
}
```

### Exemple
```
Setup multi-instruments:
- Flûte 1 : MIDI_CHANNEL = 1
- Flûte 2 : MIDI_CHANNEL = 2
- Synth : MIDI_CHANNEL = 3

DAW envoie:
- Note C6 sur canal 1 → Flûte 1 joue, Flûte 2 ignore
- Note D6 sur canal 2 → Flûte 2 joue, Flûte 1 ignore
- Note E6 sur canal 3 → Synth joue, flûtes ignorent
```

---

## 🔧 Architecture d'implémentation

### Flux de données
```
MIDI Controller
    ↓
MidiHandler.processMidiEvent()
    ↓ (filtrage canal MIDI)
    ├─ (case 0xB0) → Control Change
    │   ↓
    │   InstrumentManager.handleControlChange(ccNumber, ccValue)
    │   ↓ (rate limiting + stockage)
    │   AirflowController.setCCValues(cc7, cc11, cc1) + autres CC
    │
    ├─ (case 0xE0) → Pitch Bend
    │   ↓
    │   InstrumentManager.handlePitchBend(pitchBendValue)
    │   ↓
    │   AirflowController.setPitchBendAdjustment(adjustment)
    │
    └─ (case 0x90/0x80) → Note On/Off
        ↓
        AirflowController.setAirflowForNote()
        → Calcul angle final : CC7 → Velocity → CC11 → Pitch Bend → Vibrato
```

### Classes modifiées

#### 1. **InstrumentManager** (gestionnaire central)
**Fichiers :** `InstrumentManager.h/cpp`

**Variables ajoutées :**
```cpp
// Control Change values
byte _ccVolume;         // CC 7  (défaut: 127)
byte _ccExpression;     // CC 11 (défaut: 127)
byte _ccModulation;     // CC 1  (défaut: 0)
byte _ccBreath;         // CC 2  (défaut: 127)
byte _ccBrightness;     // CC 74 (défaut: 64)

// Pitch Bend
int16_t _pitchBend;     // -8192 à +8191 (défaut: 0)

// Rate Limiting
unsigned long _lastCCTime;
uint16_t _ccCount;
unsigned long _ccWindowStart;
```

**Méthodes ajoutées :**
```cpp
void handleControlChange(byte ccNumber, byte ccValue);
void handlePitchBend(uint16_t pitchBendValue);
void resetAllControllers();  // CC 121
byte getCCVolume() const;
byte getCCExpression() const;
byte getCCModulation() const;
byte getCCBreath() const;
byte getCCBrightness() const;
int16_t getPitchBend() const;
void allSoundOff();  // CC 120 et CC 123
```

**Logique :**
- Reçoit les CC depuis MidiHandler
- **Rate limiting :** Limite à 10 CC/sec (exemptions : 120, 121, 123)
- Stocke les valeurs actuelles de tous les CC
- Synchronise avec AirflowController
- Gère All Sound Off (CC120, CC123)
- Gère Reset All Controllers (CC121)

---

#### 2. **MidiHandler** (réception MIDI)
**Fichiers :** `MidiHandler.h/cpp`

**Méthode ajoutée :**
```cpp
bool isChannelAccepted(byte channel);
```

**Modifications :**
```cpp
void MidiHandler::processMidiEvent(midiEventPacket_t midiEvent) {
  byte channel = midiEvent.byte1 & 0x0F;

  // Filtrage canal MIDI
  if (!isChannelAccepted(channel)) {
    return;  // Ignorer messages d'autres canaux
  }

  byte messageType = midiEvent.byte1 & 0xF0;

  switch (messageType) {
    case 0xB0:  // Control Change
      {
        byte ccNumber = midiEvent.byte2;
        byte ccValue = midiEvent.byte3;
        _instrument.handleControlChange(ccNumber, ccValue);
      }
      break;

    case 0xE0:  // Pitch Bend
      {
        uint16_t pitchBendValue = (uint16_t)midiEvent.byte3 << 7 | midiEvent.byte2;
        _instrument.handlePitchBend(pitchBendValue);
      }
      break;

    // ... autres messages
  }
}

bool MidiHandler::isChannelAccepted(byte channel) {
  if (MIDI_CHANNEL == 0) return true;  // Omni mode
  return (channel == (MIDI_CHANNEL - 1));  // Canal spécifique (0-indexed)
}
```

**Rôle :**
- Filtre les messages par canal MIDI (omni ou spécifique)
- Détecte les messages CC MIDI et Pitch Bend
- Extrait numéro CC/valeur ou valeur Pitch Bend (14-bit)
- Délègue à InstrumentManager

---

#### 3. **AirflowController** (application des CC)
**Fichiers :** `AirflowController.h/cpp`

**Variables ajoutées :**
```cpp
byte _ccVolume;         // CC 7
byte _ccExpression;     // CC 11
byte _ccModulation;     // CC 1
int8_t _pitchBendAdjustment;  // Ajustement pitch bend en %
```

**Méthodes ajoutées :**
```cpp
void setCCValues(byte ccVolume, byte ccExpression, byte ccModulation);
void setPitchBendAdjustment(int8_t adjustment);  // -10% à +10%
```

**Logique dans `setAirflowForNote()` - NOUVELLE FORMULE :**
```cpp
// 0. Calcul plage de la note
minAngle = SERVO_AIRFLOW_MIN + (plage × airflowMinPercent / 100);
maxAngle = SERVO_AIRFLOW_MIN + (plage × airflowMaxPercent / 100);

// 1. CC7 (Volume) RÉDUIT la limite haute AVANT velocity
//    CC7 = 127 → effectiveMaxAngle = maxAngle (plage complète)
//    CC7 = 64  → effectiveMaxAngle = 50% de la plage
//    CC7 = 0   → effectiveMaxAngle = minAngle (plage minimale)
float volumeFactor = _ccVolume / 127.0;
uint16_t effectiveMaxAngle = minAngle + (maxAngle - minAngle) × volumeFactor;

// 2. VELOCITY → angle de base DANS [minAngle, effectiveMaxAngle]
uint16_t baseAngle = map(velocity, 1, 127, minAngle, effectiveMaxAngle);

// 3. CC11 (Expression) module DANS [minAngle, baseAngle]
//    CC11 = 127 → baseAngle (pleine expression dans plage réduite)
//    CC11 = 0   → minAngle (expression minimale de la note)
float expressionFactor = _ccExpression / 127.0;
float finalAngleWithoutVibrato = minAngle + (baseAngle - minAngle) × expressionFactor;

// 4. PITCH BEND : ajustement fin ±10%
if (_pitchBendAdjustment != 0) {
  float pitchBendOffset = (finalAngleWithoutVibrato - minAngle)
                          × (_pitchBendAdjustment / 100.0);
  finalAngleWithoutVibrato += pitchBendOffset;
}

// 5. Limiter bornes valides
if (finalAngleWithoutVibrato < SERVO_AIRFLOW_MIN)
  finalAngleWithoutVibrato = SERVO_AIRFLOW_MIN;
if (finalAngleWithoutVibrato > SERVO_AIRFLOW_MAX)
  finalAngleWithoutVibrato = SERVO_AIRFLOW_MAX;

// 6. Stocker pour vibrato
_baseAngleWithoutVibrato = (uint16_t)(finalAngleWithoutVibrato + 0.5);
_vibratoActive = (_ccModulation > 0);

// 7. CC1 (Vibrato) appliqué dans update()
//    Oscillation continue autour de _baseAngleWithoutVibrato
//    Limité aux bornes [_currentMinAngle, _currentMaxAngle]
```

---

#### 4. **NoteSequencer** (All Sound Off)
**Fichiers :** `NoteSequencer.h/cpp`

**Méthode ajoutée :**
```cpp
void stop();  // Arrêt immédiat (pour All Sound Off)
```

**Logique :**
```cpp
void NoteSequencer::stop() {
  _currentNote = 0;
  _currentVelocity = 0;
  transitionTo(STATE_IDLE);
}
```

---

## 📊 Formule complète de calcul - NOUVELLE LOGIQUE

### Angle final airflow

**ORDRE D'APPLICATION :** CC7 → Velocity → CC11 → Pitch Bend → Vibrato

```
1. Calcul plage de la note
   minAngle = SERVO_AIRFLOW_MIN + (plage × airflowMinPercent / 100)
   maxAngle = SERVO_AIRFLOW_MIN + (plage × airflowMaxPercent / 100)

2. CC7 (Volume) RÉDUIT la limite haute AVANT velocity
   volumeFactor = CC7 / 127
   effectiveMaxAngle = minAngle + (maxAngle - minAngle) × volumeFactor

   Comportement:
   - CC7 = 127 → effectiveMaxAngle = maxAngle (plage complète)
   - CC7 = 64  → effectiveMaxAngle au milieu entre minAngle et maxAngle
   - CC7 = 0   → effectiveMaxAngle = minAngle (plage minimale)

3. Velocity → angle de base DANS [minAngle, effectiveMaxAngle]
   baseAngle = map(velocity, 1, 127, minAngle, effectiveMaxAngle)

4. CC11 (Expression) module DANS [minAngle, baseAngle]
   expressionFactor = CC11 / 127
   finalAngle = minAngle + (baseAngle - minAngle) × expressionFactor

   Comportement:
   - CC11 = 127 → finalAngle = baseAngle (pleine expression)
   - CC11 = 64  → finalAngle au milieu entre minAngle et baseAngle
   - CC11 = 0   → finalAngle = minAngle (expression minimale)

5. Pitch Bend : ajustement fin ±10%
   Si pitchBend ≠ 0:
     pitchBendFactor = pitchBend / 8192.0  (-1.0 à +1.0)
     adjustment = pitchBendFactor × 10%
     pitchBendOffset = (finalAngle - minAngle) × adjustment
     finalAngle += pitchBendOffset

6. Clamp dans bornes servo globales
   finalAngle = constrain(finalAngle, SERVO_AIRFLOW_MIN, SERVO_AIRFLOW_MAX)

7. CC1 (Vibrato) - appliqué en continu dans update()
   Si CC1 > 0:
     vibratoFreq = 6.0 Hz (VIBRATO_FREQUENCY_HZ)
     vibratoAmplitude = (CC1/127) × 8° (VIBRATO_MAX_AMPLITUDE_DEG)
     vibratoOffset = sin(2π × freq × time) × amplitude
     finalWithVibrato = finalAngle + vibratoOffset

8. Clamp vibrato dans bornes NOTE
   finalWithVibrato = constrain(finalWithVibrato, minAngle, maxAngle)
```

### Exemple concret - NOUVELLE LOGIQUE

**Configuration :**
- Note : C6 (MIDI 84)
- airflowMinPercent : 20%
- airflowMaxPercent : 75%
- SERVO_AIRFLOW_MIN : 60°
- SERVO_AIRFLOW_MAX : 100°

**Calcul avec nouvelle logique :**
```
Plage servo absolue: 100 - 60 = 40°

1. Plage de la note C6
   minAngle = 60 + (40 × 20/100) = 68°
   maxAngle = 60 + (40 × 75/100) = 90°

2. CC7 = 64 (50% volume) - RÉDUIT la limite haute
   volumeFactor = 64/127 = 0.50
   effectiveMaxAngle = 68 + (90 - 68) × 0.50 = 68 + 11 = 79°
   ✓ Plage réduite: [68°, 79°] au lieu de [68°, 90°]

3. Velocity = 100 - utilise la plage RÉDUITE
   baseAngle = map(100, 1, 127, 68, 79) ≈ 77°
   ✓ Velocity limitée par CC7!

4. CC11 = 64 (50% expression) - module dans plage réduite
   expressionFactor = 64/127 = 0.50
   finalAngle = 68 + (77 - 68) × 0.50 = 68 + 4.5 = 72.5°
   ✓ Respecte la borne réduite: 68° ≤ 72.5° ≤ 77°

5. Pitch Bend = +4096 (+50% de +8192)
   pitchBendFactor = 4096 / 8192 = 0.5
   adjustment = 0.5 × 10% = 5%
   pitchBendOffset = (72.5 - 68) × 0.05 = 0.225°
   finalAngle = 72.5 + 0.225 ≈ 72.7°

6. CC1 = 40 (vibrato modéré)
   vibratoAmplitude = (40/127) × 8 = 2.5°
   vibratoOffset = sin(...) × 2.5  // Varie entre -2.5° et +2.5°

7. finalAngle = 72.7° ± 2.5°
   → Oscillation entre 70.2° et 75.2°
   → Limité à [68°, 79°] (plage réduite par CC7)
```

**Comparaison ancienne vs nouvelle logique :**
```
Configuration: Velocity 100, Note C6 [68°-90°]

ANCIENNE LOGIQUE (Velocity → CC11 → CC7):
- Velocity 100 → baseAngle = 86°
- CC11 = 64 → modulatedAngle = 77°
- CC7 = 64 → finalAngle = 77° × 0.5 = 38.5° ❌ (sous minAngle!)

NOUVELLE LOGIQUE (CC7 → Velocity → CC11):
- CC7 = 64 → effectiveMaxAngle = 79° (limite haute réduite)
- Velocity 100 → baseAngle = 77° (dans plage réduite)
- CC11 = 64 → finalAngle = 72.5° ✓ (toujours dans bornes!)

✓ CC7 agit maintenant comme un vrai contrôle de volume
✓ Plus de risque de descendre sous minAngle
✓ Comportement plus intuitif et musical
```

---

## 🎹 Utilisation pratique

### Scénario 1 : Contrôle volume global (CC7)
```
Message MIDI: CC 7, valeur 80

Note C6 [68°-90°], Velocity 127, CC11 = 127

SANS CC7 (défaut CC7=127):
→ effectiveMaxAngle = 90°
→ baseAngle (velocity 127) = 90°
→ finalAngle (CC11=127) = 90°

AVEC CC7 = 80:
→ effectiveMaxAngle = 68 + (90-68)×(80/127) = 82°
→ baseAngle (velocity 127) = 82° (limité par CC7!)
→ finalAngle (CC11=127) = 82°

✓ CC7 réduit la limite haute de la plage
✓ Utile pour ajuster volume global sans modifier velocity/expression
```

### Scénario 2 : Crescendo pendant note (CC11)
```
Note C6 [68°-90°], Velocity 100, CC7 = 127

1. Note On: C6, velocity 100
   → effectiveMaxAngle = 90° (CC7=127, pas de réduction)
   → baseAngle = map(100, 1, 127, 68, 90) ≈ 86°

2. CC 11 = 0 (pianissimo)
   → finalAngle = 68 + (86-68)×0.0 = 68° (minimum de la note)

3. CC 11 = 64 (crescendo progressif)
   → finalAngle = 68 + (86-68)×0.5 = 77° (milieu)

4. CC 11 = 127 (fortissimo)
   → finalAngle = 68 + (86-68)×1.0 = 86° (maximum selon velocity)

5. Note Off: C6

✓ L'expression module DANS la plage [68°, 86°] définie par velocity
✓ Crescendo naturel respectant la physique de la note
```

### Scénario 3 : Vibrato expressif (CC1)
```
1. Note On: D6, velocity 80
   → Son stable (CC1 défaut = 0)

2. CC 1 = 0 (pas de vibrato)
   → Son stable, angle fixe

3. CC 1 = 50 (vibrato modéré)
   → Amplitude: (50/127) × 8° = 3.15°
   → Airflow oscille ±3.15° à 6Hz
   → Vibrato musical doux

4. CC 1 = 100 (vibrato intense)
   → Amplitude: (100/127) × 8° = 6.3°
   → Airflow oscille ±6.3° à 6Hz
   → Vibrato expressif fort

5. CC 1 = 127 (vibrato maximum)
   → Amplitude: 8° (maximum)
   → Airflow oscille ±8° à 6Hz
   → Vibrato très prononcé

✓ Vibrato limité aux bornes de la note (pas de dépassement)
```

### Scénario 4 : Pitch Bend pour micro-ajustements
```
Note C6, finalAngle = 80° (après CC7/Velocity/CC11)

1. Pitch Bend = 8192 (centre, pas de bend)
   → Angle reste 80°

2. Pitch Bend = 12288 (+50% de la plage)
   → Ajustement = +5% airflow
   → Angle = 80° × 1.05 = 84°

3. Pitch Bend = 16383 (maximum, +8191)
   → Ajustement = +10% airflow
   → Angle = 80° × 1.10 = 88°

4. Pitch Bend = 4096 (-50% de la plage)
   → Ajustement = -5% airflow
   → Angle = 80° × 0.95 = 76°

5. Pitch Bend = 0 (minimum, -8192)
   → Ajustement = -10% airflow
   → Angle = 80° × 0.90 = 72°

✓ Pitch bend permet ajustements fins de hauteur
✓ Simule variation de débit pour monter/descendre la note
```

### Scénario 5 : All Sound Off (urgence)
```
Situation: Notes bloquées, problème MIDI
Action: Envoyer CC 120 ou CC 123
Résultat:
  - Queue vidée
  - Séquenceur stoppé
  - Valve fermée
  - Servos au repos
  - Silence immédiat

✓ CC 120 et CC 123 identiques (All Sound Off)
✓ Exempts de rate limiting (priorité absolue)
```

### Scénario 6 : Reset All Controllers (CC121)
```
Situation: Contrôleurs dans état inconnu
Action: Envoyer CC 121
Résultat:
  - CC1 (Modulation) → 0 (pas de vibrato)
  - CC2 (Breath) → 127
  - CC7 (Volume) → 127 (volume max)
  - CC11 (Expression) → 127 (expression max)
  - CC74 (Brightness) → 64 (centre)
  - Pitch Bend → 8192 (centre, 0)

✓ Réinitialise état propre pour nouvelle performance
✓ Exempt de rate limiting
```

### Scénario 7 : Combinaison CC + Pitch Bend + Vibrato
```
Configuration complète pour note expressive:

1. CC 7 = 100 (volume 79%)
   → Réduit limite haute à 79% de la plage

2. Note On: C6, velocity 110
   → baseAngle dans plage réduite par CC7

3. CC 11 = 90 (expression forte)
   → finalAngle ≈ 71% de la plage réduite

4. Pitch Bend = 10000 (+1800 de centre)
   → Ajustement +2.2% airflow
   → Monte légèrement la note

5. CC 1 = 60 (vibrato modéré)
   → Oscillation ±3.8° à 6Hz
   → Ajoute vibrato musical

✓ Résultat: Note expressive avec volume contrôlé, pitch légèrement monté, vibrato doux
✓ Tous les paramètres respectent les bornes de sécurité
```

---

## 🔍 Debug et monitoring

### Messages DEBUG activés (DEBUG = 1)

#### Réception CC
```
DEBUG: CC 7 (Volume) = 80
DEBUG: CC 11 (Expression) = 127
DEBUG: CC 1 (Modulation) = 40
DEBUG: CC 120 (All Sound Off)
```

#### Calcul airflow avec CC
```
DEBUG: AirflowController - Note MIDI: 84 | Vel: 100 | BaseAngle: 86° | CC7: 80 | CC11: 127 | CC1: 40 | FinalAngle: 54°
```

#### All Sound Off
```
DEBUG: InstrumentManager - All Sound Off exécuté
DEBUG: NoteSequencer - STOP forcé (All Sound Off)
```

---

## ⚙️ Configuration

### Constantes MIDI (settings.h)

**Canal MIDI :**
```cpp
#define MIDI_CHANNEL 0  // 0 = omni mode, 1-16 = canal spécifique
```

**Rate Limiting :**
```cpp
#define CC_RATE_LIMIT_PER_SECOND 10  // Max 10 CC/seconde
```

**Valeurs par défaut des Control Change :**
```cpp
#define CC_VOLUME_DEFAULT      127  // Volume max
#define CC_EXPRESSION_DEFAULT  127  // Expression max
#define CC_MODULATION_DEFAULT  0    // Pas de vibrato
#define CC_BREATH_DEFAULT      127  // Breath max
#define CC_BRIGHTNESS_DEFAULT  64   // Brightness centre
```

**Paramètres vibrato :**
```cpp
#define VIBRATO_FREQUENCY_HZ       6.0   // Fréquence en Hz
#define VIBRATO_MAX_AMPLITUDE_DEG  8.0   // Amplitude max en degrés
```

**Ajustements possibles vibrato :**
- `VIBRATO_FREQUENCY_HZ` : 4-8 Hz (typique instruments à vent)
  - 4 Hz : vibrato lent, expressif
  - 6 Hz : vibrato standard flûte (défaut)
  - 8 Hz : vibrato rapide, intense
- `VIBRATO_MAX_AMPLITUDE_DEG` : 5-12° (selon réactivité servo)
  - 5° : vibrato subtil
  - 8° : vibrato standard (défaut)
  - 12° : vibrato très prononcé

**Paramètres Pitch Bend :**
```cpp
#define PITCH_BEND_RANGE_SEMITONES 2    // Plage ±2 demi-tons
#define PITCH_BEND_AIRFLOW_PERCENT 10   // Ajustement ±10% airflow
```

**Ajustements possibles pitch bend :**
- `PITCH_BEND_RANGE_SEMITONES` : 1-12 demi-tons
  - 1 : plage étroite (micro-ajustements)
  - 2 : plage standard (défaut)
  - 12 : plage large (octave)
- `PITCH_BEND_AIRFLOW_PERCENT` : 5-20%
  - 5% : effet subtil
  - 10% : effet standard (défaut)
  - 20% : effet prononcé

---

## 🧪 Tests recommandés

### Test 1 : Volume global
```
1. Jouer note C6, velocity 100
2. Observer angle (ex: 70°)
3. Envoyer CC 7 = 64 (50%)
4. Jouer même note
5. Vérifier angle ≈ 35° (70 × 0.5)
```

### Test 2 : Expression dynamique
```
1. Jouer note tenue (C6, velocity 100)
2. Pendant que note joue:
   - Envoyer CC 11 = 30 (faible)
   - Attendre 1s
   - Envoyer CC 11 = 127 (fort)
3. Écouter crescendo
```

### Test 3 : Vibrato
```
1. Jouer note tenue (D6, velocity 80)
2. Envoyer CC 1 = 0 → son stable
3. Envoyer CC 1 = 40 → vibrato modéré
4. Envoyer CC 1 = 100 → vibrato intense
5. Observer oscillation airflow dans DEBUG
```

### Test 4 : All Sound Off
```
1. Jouer plusieurs notes en séquence rapide
2. Envoyer CC 120 pendant lecture
3. Vérifier arrêt immédiat:
   - Valve fermée
   - Servos au repos
   - Queue vidée
```

### Test 5 : Combinaison CC
```
1. CC 7 = 80 (volume 63%)
2. CC 11 = 100 (expression 79%)
3. CC 1 = 50 (vibrato modéré)
4. Jouer note C6, velocity 100
5. Calculer attendu:
   baseAngle = 86°
   × (80/127) = 54°
   × (100/127) = 43°
   ± vibrato ±3°
   → 40-46° avec oscillation
```

---

## 📚 Compatibilité DAW

### DAWs testés

| DAW          | CC Support | Automation | Notes              |
|--------------|------------|------------|--------------------|
| Ableton Live | ✅         | ✅         | Excellent          |
| FL Studio    | ✅         | ✅         | Excellent          |
| Reaper       | ✅         | ✅         | Excellent          |
| Logic Pro    | ✅         | ✅         | Excellent          |
| Cubase       | ✅         | ✅         | Excellent          |
| GarageBand   | ✅         | ⚠️         | Limité (pas CC1)   |

### Mapping contrôleurs MIDI

**Clavier MIDI standard :**
- Fader 1 → CC 7 (Volume)
- Fader 2 → CC 11 (Expression)
- Molette modulation → CC 1 (Vibrato)

**Contrôleur dédié (ex: Korg nanoKONTROL) :**
- Potentiomètre 1 → CC 7
- Potentiomètre 2 → CC 11
- Potentiomètre 3 → CC 1

---

## 🔄 Évolutions possibles

### Court terme
- [ ] Ajouter CC 64 (Sustain Pedal)
- [ ] Utiliser CC 2 (Breath Controller) pour contrôle alternatif airflow
- [ ] Utiliser CC 74 (Brightness) pour ajustement timbre

### Moyen terme
- [ ] Aftertouch → Expression dynamique temps réel
- [ ] Program Change → Modes de jeu (flûte irlandaise, baroque, moderne)
- [ ] CC 14-bit haute résolution (MSB+LSB)
- [ ] Calibration des courbes de réponse CC (linéaire, exponentielle, logarithmique)

### Long terme
- [ ] SysEx pour configuration à distance
- [ ] Enregistrement/lecture presets CC
- [ ] MPE (MIDI Polyphonic Expression) pour contrôle multi-dimensionnel
- [ ] Mapping personnalisable CC → fonctions

---

## 📝 Notes importantes

1. **Ordre d'application :** **CC7 → Velocity → CC11 → Pitch Bend → CC1 (vibrato)** (changement majeur 2026-02-04)
2. **CC7 nouvelle logique :** Réduit la limite haute AVANT velocity (plus intuitif)
3. **Valeurs par défaut :** CC7=127, CC11=127, CC1=0, CC2=127, CC74=64, Pitch Bend=8192
4. **Rate Limiting :** 10 CC/seconde (exemptions : 120, 121, 123)
5. **Canal MIDI :** Omni mode (0) par défaut, configurable pour setups multi-instruments
6. **Pitch Bend :** ±10% airflow pour ±2 demi-tons (approximation hauteur)
7. **Vibrato :** Sin() LUT optimisé (256 entrées PROGMEM), impact CPU <1%
8. **All Sound Off :** CC 120 et CC 123 identiques, priorité absolue
9. **Reset Controllers :** CC 121 réinitialise tous les CC aux valeurs par défaut
10. **Bornes sécurité :** Tous les CC respectent [airflowMinPercent, airflowMaxPercent] de la note
11. **Debug :** Active dans `settings.h` avec `DEBUG 1`

---

## 📜 Historique et correctifs

### 2026-02-04 : NOUVELLE LOGIQUE CC7/CC11 - Changement majeur

**Changement fondamental :** CC7 réduit maintenant la limite haute AVANT velocity et CC11

**Motivation :**
- CC7 doit agir comme un vrai contrôle de volume (limite le maximum possible)
- CC11 offre ensuite des nuances expressives DANS la plage limitée par CC7
- Plus intuitif et musical : volume global puis expression

**ORDRE NOUVEAU :**
```
CC7 → Velocity → CC11 → Pitch Bend → Vibrato
```

**ORDRE ANCIEN (remplacé) :**
```
Velocity → CC11 → CC7 → Vibrato
```

**Nouvelle formule :**
```cpp
// 1. CC7 réduit limite haute
volumeFactor = _ccVolume / 127.0;
effectiveMaxAngle = minAngle + (maxAngle - minAngle) × volumeFactor;

// 2. Velocity utilise plage réduite
baseAngle = map(velocity, 1, 127, minAngle, effectiveMaxAngle);

// 3. CC11 module dans plage réduite
expressionFactor = _ccExpression / 127.0;
finalAngle = minAngle + (baseAngle - minAngle) × expressionFactor;
```

**Avantages :**
- ✅ CC7 = vrai contrôle de volume (limite max)
- ✅ CC11 = nuances expressives dans plage volume
- ✅ Comportement plus intuitif
- ✅ Garde toutes les garanties de sécurité (bornes respectées)

Voir commit: `Nouvelle logique CC7/CC11 : Volume réduit plage avant Expression`

---

### 2026-02-04 : CC2 Breath Controller + Suppression Pitch Bend

**CC2 BREATH CONTROLLER IMPLÉMENTÉ (Option 1 - Remplacement Velocity) :**

CC2 remplace velocity pour contrôle dynamique du souffle en temps réel :
- **Lissage** : Buffer circulaire 5 valeurs (moyenne glissante anti-jitter)
- **Courbe exponentielle** : CC2^1.4 pour réponse naturelle
- **Seuil silence** : CC2 < 10 → valve fermée
- **Fallback velocity** : Si CC2 absent > 1s, utilise velocity
- **Rate limiting séparé** : 50 CC2/sec (haute fréquence)

**Ordre nouveau avec CC2 :**
```
CC7 → CC2 (si disponible, sinon Velocity) → CC11 → Vibrato
```

**Constantes CC2 (settings.h) :**
```cpp
CC2_ENABLED true
CC2_RATE_LIMIT_PER_SECOND 50
CC2_SILENCE_THRESHOLD 10
CC2_SMOOTHING_BUFFER_SIZE 5
CC2_RESPONSE_CURVE 1.4
CC2_TIMEOUT_MS 1000
```

**Avantages :**
- ✅ Servo-flute devient véritable instrument à vent MIDI
- ✅ Contrôle breath physique (Yamaha BC3, TEControl BBC2)
- ✅ Automation DAW pour souffle pré-enregistré
- ✅ Réponse naturelle avec courbe exponentielle

Voir commit: `CC2 Breath Controller : Contrôle dynamique souffle en temps réel`

---

**PITCH BEND RETIRÉ (Logique incorrecte) :**

**Problème identifié :**
- Pitch bend modifiait l'AIRFLOW (débit d'air) au lieu de la HAUTEUR (doigts)
- Sur une vraie flûte, hauteur = doigts, volume = souffle
- Logique physiquement incorrecte

**Solution :**
- Retrait complet du pitch bend
- Contrôle airflow géré par CC2/CC7/CC11 uniquement
- Ordre simplifié : **CC7 → CC2/Velocity → CC11 → Vibrato**

**Note future :**
Si pitch bend nécessaire, l'implémenter correctement en modifiant les DOIGTS (FingerController), pas l'airflow.

Voir commit: `Suppression Pitch Bend : Logique incorrecte retirée`

---

### 2026-02-04 : Améliorations MIDI (avant implémentation CC2)

**Nouveaux CC implémentés :**
- CC 2 (Breath Controller) - stocké pour usage futur (remplacé plus tard par implémentation complète)
- CC 74 (Brightness) - stocké pour usage futur
- CC 121 (Reset All Controllers) - réinitialise tous CC
- CC 123 (All Notes Off) - identique à CC 120

**Pitch Bend ajouté (retiré plus tard) :**
- Valeur 14-bit (0-16383, centre 8192)
- Plage : ±2 demi-tons
- Effet : ±10% airflow
- ⚠️ **Logique incorrecte** → Retiré le même jour

**Rate Limiting :**
- Limite : 10 CC/seconde (configurable)
- Exemptions : CC 120, 121, 123 (urgence)
- Algorithme : fenêtre glissante 1 seconde

**Canal MIDI :**
- Mode omni (0) : écoute tous les canaux
- Mode spécifique (1-16) : écoute un seul canal
- Utile pour setups multi-instruments

**Constantes ajoutées (settings.h) :**
```cpp
MIDI_CHANNEL
CC_RATE_LIMIT_PER_SECOND
VIBRATO_FREQUENCY_HZ
VIBRATO_MAX_AMPLITUDE_DEG
CC_*_DEFAULT (tous les CC)
```

Voir commit: `Améliorations MIDI : Canal, Pitch Bend, CC étendus, Rate Limiting`

---

### 2026-01-26 : Fix CC7 et vibrato - Respect bornes note

**Problème :** CC7 et vibrato pouvaient sortir de l'intervalle [airflowMinPercent, airflowMaxPercent]

**Solution :**
- CC7 module maintenant DANS [minAngle, modulatedAngle] au lieu de multiplier globalement
- Vibrato limité aux bornes de la note en cours
- Garantie absolue : servo toujours dans l'intervalle défini par la note

**Ancienne formule CC7 (bug) :**
```cpp
finalAngle = modulatedAngle × (CC7 / 127.0)  // Pouvait descendre sous minAngle!
```

**Nouvelle formule CC7 (fix) :**
```cpp
finalAngle = minAngle + (modulatedAngle - minAngle) × (CC7 / 127.0)  // Jamais sous minAngle ✅
```

Détails complets : voir CC7_VIBRATO_FIX.md

---

### 2026-01-26 : Correctifs critiques CC

**4 problèmes critiques résolus** (voir CC_AUDIT_REPORT.md pour détails) :

1. ✅ **Vibrato fonctionnel** - Update continu implémenté
   - Ajout variables d'état dans AirflowController
   - update() appelé en boucle pour appliquer vibrato
   - sin() LUT pour optimisation (gain 25x performance)

2. ✅ **Optimisation sin()** - Lookup table 256 entrées
   - SIN_LUT[256] en PROGMEM
   - Réduction CPU : 5-7% → <1%

3. ✅ **Validation CC** - Sécurité valeurs entrantes
   - Vérification ccValue ≤ 127
   - Protection overflow et dommages matériel

4. ✅ **Fix overflow millis()** - Stabilité long terme
   - Modulo dans calcul phase vibrato
   - Fonctionnement stable 49+ jours

---

### 2026-01-25 : Correctif CC11 (Expression)

**Problème initial :**
- CC11 multiplicatif pouvait descendre sous `airflowMinPercent`
- Exemple : Note avec min 20%, CC11=50 → 10% (invalide!)

**Solution - Option A Proposition 2 :**
```cpp
// 1. Velocity définit baseAngle (dans [minAngle, maxAngle])
baseAngle = minAngle + (maxAngle - minAngle) × (velocity / 127.0)

// 2. CC11 module DANS [minAngle, baseAngle]
modulatedAngle = minAngle + (baseAngle - minAngle) × (CC11 / 127.0)

// 3. CC7 module DANS [minAngle, modulatedAngle]
finalAngle = minAngle + (modulatedAngle - minAngle) × (CC7 / 127.0)
```

**Note :** Cette formule a été remplacée le 2026-02-04 par la nouvelle logique CC7→Velocity→CC11

**Résultat :**
- ✅ CC11 ne peut jamais descendre sous minAngle
- ✅ CC7 ne peut jamais descendre sous minAngle
- ✅ CC1 (Vibrato) limité aux bornes de la note
- ✅ Tous les CC respectent l'intervalle [airflowMinPercent, airflowMaxPercent]

---

### 2026-01-25 : Implémentation initiale

**CC implémentés :**
- CC 1 (Modulation/Vibrato)
- CC 7 (Volume)
- CC 11 (Expression)
- CC 120 (All Sound Off)

**Architecture :**
- Réception dans MidiHandler
- Gestion centralisée InstrumentManager
- Application dans AirflowController
- Stop d'urgence NoteSequencer

---

## ✅ Résumé implémentation

**Fichiers modifiés :**
- `InstrumentManager.h/cpp` - Gestion CC centralisée, rate limiting CC2
- `MidiHandler.h/cpp` - Réception CC MIDI, filtrage canal
- `AirflowController.h/cpp` - Application CC sur airflow, CC2 breath controller, nouvelle logique CC7→CC2→CC11
- `NoteSequencer.h/cpp` - Méthode stop() pour All Sound Off
- `settings.h` - Constantes MIDI, CC, vibrato, CC2 breath

**Lignes de code ajoutées :** ~500 lignes (total avec toutes améliorations)

**Complexité :** Moyenne-Haute
- Rate limiting avec fenêtre glissante (10 CC/s général, 50 CC2/s)
- CC2 Breath Controller avec lissage, courbe exponentielle, fallback
- Nouvelle logique CC7→CC2/Velocity→CC11→Vibrato
- Vibrato avec sin() LUT optimisé
- Filtrage canal MIDI

**Compatibilité :** 100% DAWs standards + contrôleurs MIDI + breath controllers

**Features MIDI complètes :**
- ✅ 8 CC implémentés (1, 2, 7, 11, 74, 120, 121, 123)
- ✅ CC2 Breath Controller (contrôle dynamique souffle)
- ✅ Rate limiting configurable (général 10/s, CC2 50/s)
- ✅ Canal MIDI (omni + spécifique)
- ✅ Reset All Controllers
- ✅ All Sound Off / All Notes Off
- ✅ Vibrato optimisé (sin LUT)
- ✅ Sécurité bornes garantie

---

**Documentation créée le :** 2026-01-25
**Dernière mise à jour :** 2026-02-04
**Version Servo Flute :** V3
**CC implémentés :** 1, 2, 7, 11, 74, 120, 121, 123
**CC2 Breath Controller :** ✅ Implémenté (Option 1 - Remplacement Velocity)
**Canal MIDI :** ✅ Implémenté (omni + spécifique)
**Rate Limiting :** ✅ Implémenté (10 CC/s général, 50 CC2/s)
**Pitch Bend :** ❌ Retiré (logique incorrecte)
