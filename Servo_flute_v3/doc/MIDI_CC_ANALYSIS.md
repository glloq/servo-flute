# Analyse : Gestion Volume et Control Change MIDI

## 📊 État actuel du système

### Gestion actuelle de la vélocité (volume)
```
NoteOn(note, velocity)
    ↓
Velocity (1-127) mappée directement sur angle servo airflow
    ↓
angle = map(velocity, 1, 127, minAngle, maxAngle)
```

**Où minAngle/maxAngle sont calculés depuis :**
- `airflowMinPercent` et `airflowMaxPercent` de chaque note
- Appliqués sur `[SERVO_AIRFLOW_MIN, SERVO_AIRFLOW_MAX]`

**Limitations actuelles :**
- ✓ Velocity gère le volume par note
- ✗ Pas de contrôle global du volume
- ✗ Pas de modification dynamique pendant le jeu
- ✗ Pas de contrôle expressif (vibrato, crescendo, etc.)
- ✗ Control Change (CC) non implémentés

---

## 🎛️ Control Change MIDI standards applicables

### CC prioritaires pour instruments à vent

#### **CC 7 : Volume (Channel Volume)**
- **Standard MIDI :** Contrôle du volume global du canal
- **Valeur :** 0-127
- **Usage flûte :**
  - Multiplicateur global sur tous les airflow
  - Permet crescendo/diminuendo sans changer velocity de chaque note
  - Joué en temps réel pendant une performance

**Formule proposée :**
```
angleCalculé = map(velocity, 1, 127, minAngle, maxAngle)
angleFinal = angleCalculé × (CC7 / 127)
```

**Exemple :**
- Note C6, velocity 100 → angle = 70°
- CC7 = 64 (50%) → angleFinal = 35°
- CC7 = 127 (100%) → angleFinal = 70°

---

#### **CC 11 : Expression**
- **Standard MIDI :** Contrôle d'expression (sous-volume)
- **Valeur :** 0-127
- **Usage flûte :**
  - Expression dynamique PENDANT la note
  - Swells (crescendo/diminuendo sur une note tenue)
  - Plus fin que CC7

**Différence CC7 vs CC11 :**
- **CC7** = Volume "master" (reste constant pendant morceau)
- **CC11** = Expression instantanée (varie constamment)

**Formule proposée :**
```
angleBase = map(velocity, 1, 127, minAngle, maxAngle)
angleVolume = angleBase × (CC7 / 127)
angleFinal = angleVolume × (CC11 / 127)
```

---

#### **CC 1 : Modulation**
- **Standard MIDI :** Roue de modulation
- **Valeur :** 0-127
- **Usage flûte :**
  - **Option A : Vibrato** (variation périodique airflow)
  - **Option B : Contrôle ouverture doigts** (demi-trous simulés)

**Implémentation vibrato :**
```
vibratoDepth = CC1  // Profondeur (0-127)
vibratoFreq = 5-7 Hz  // Fréquence typique vibrato flûte

offset = sin(millis() × vibratoFreq) × (vibratoDepth / 127) × amplitude
angleFinal = angleBase + offset
```

---

#### **CC 2 : Breath Controller**
- **Standard MIDI :** Contrôleur de souffle (breath controller hardware)
- **Valeur :** 0-127
- **Usage flûte :**
  - Si utilisateur a un breath controller USB
  - Remplace ou complète la velocity
  - Contrôle temps-réel très naturel

**Formule proposée :**
```
Si CC2 actif (> 0) :
    airflow = CC2 (remplace velocity)
Sinon :
    airflow = velocity (comportement actuel)
```

---

#### **CC 74 : Brightness / Timbre**
- **Standard MIDI :** Contrôle de brillance
- **Valeur :** 0-127
- **Usage flûte :**
  - Ajuste la "dureté" du souffle
  - CC74 faible → son doux (airflow réduit)
  - CC74 fort → son brillant (airflow augmenté)

**Formule proposée :**
```
timbreMultiplier = 0.7 + (CC74 / 127) × 0.6
// Range : 0.7 (son doux) à 1.3 (son brillant)

angleFinal = angleBase × timbreMultiplier
```

---

#### **CC 64 : Sustain Pedal**
- **Standard MIDI :** Pédale de sustain
- **Valeur :** 0-63 (off), 64-127 (on)
- **Usage flûte :**
  - Garde la valve air ouverte même après NoteOff
  - Simule respiration continue
  - Notes se chevauchent naturellement

**Implémentation :**
```
Sur NoteOff :
    Si CC64 >= 64 :
        Ne pas fermer valve solénoïde
        Attendre CC64 < 64 pour fermer
```

---

#### **CC 120 : All Sound Off**
- **Standard MIDI :** Couper tout son immédiatement
- **Usage flûte :**
  - Urgence : ferme valve + tous servos position repos
  - Utile en cas d'erreur MIDI

---

#### **CC 121 : Reset All Controllers**
- **Standard MIDI :** Reset tous CC à défaut
- **Usage flûte :**
  - CC7 → 127 (volume max)
  - CC11 → 127 (expression max)
  - CC1 → 0 (pas de modulation)
  - CC74 → 64 (timbre neutre)

---

### CC secondaires (optionnels)

#### **CC 5 : Portamento Time**
- Temps de glissement entre notes
- Utile pour flûte traversière (pitch bend + airflow transition)
- Complexe à implémenter avec servos

#### **CC 65 : Portamento On/Off**
- Active/désactive le portamento

#### **CC 84 : Portamento Control**
- Note source du portamento

---

## 🏗️ Architecture d'implémentation

### Option 1 : CC multiplicateurs simples (RECOMMANDÉ)

**Architecture :**
```
MidiHandler (reçoit CC)
    ↓
InstrumentManager (stocke valeurs CC actuelles)
    ↓
AirflowController (applique multiplicateurs)
```

**Variables globales à ajouter :**
```cpp
class InstrumentManager {
  byte _ccVolume;         // CC7  (défaut: 127)
  byte _ccExpression;     // CC11 (défaut: 127)
  byte _ccModulation;     // CC1  (défaut: 0)
  byte _ccBreath;         // CC2  (défaut: 0)
  byte _ccBrightness;     // CC74 (défaut: 64)
  bool _sustainPedal;     // CC64 (défaut: false)
};
```

**Calcul final angle airflow :**
```cpp
uint16_t AirflowController::calculateFinalAngle(byte velocity, byte midiNote) {
  // 1. Angle base depuis note + velocity
  uint16_t baseAngle = mapVelocityToAngle(velocity, midiNote);

  // 2. Appliquer CC7 (Volume)
  float angle = baseAngle * (_ccVolume / 127.0);

  // 3. Appliquer CC11 (Expression)
  angle *= (_ccExpression / 127.0);

  // 4. Appliquer CC74 (Brightness)
  float brightness = 0.7 + (_ccBrightness / 127.0) * 0.6;
  angle *= brightness;

  // 5. Appliquer CC2 (Breath) si actif
  if (_ccBreath > 0) {
    angle = mapBreathToAngle(_ccBreath, midiNote);
  }

  // 6. Appliquer CC1 (Vibrato) si actif
  if (_ccModulation > 0) {
    angle += calculateVibrato(_ccModulation);
  }

  // 7. Clamp dans les limites
  return constrain(angle, SERVO_AIRFLOW_MIN, SERVO_AIRFLOW_MAX);
}
```

**Avantages :**
- ✓ Simple à implémenter
- ✓ Pas de changement majeur architecture
- ✓ Compatible avec système actuel
- ✓ Performances légères

**Inconvénients :**
- ✗ Vibrato basique (pas de contrôle fin)
- ✗ Pas de portamento

---

### Option 2 : CC avec state machine avancée

**Architecture :**
```
MidiHandler
    ↓
CCProcessor (nouvelle classe)
    ↓ Calcule modificateurs
InstrumentManager
    ↓
AirflowController (reçoit angle final)
```

**Classe CCProcessor :**
```cpp
class CCProcessor {
  // Gère vibrato, portamento, enveloppes
  float calculateModulation(unsigned long time);
  float calculatePortamento(byte fromNote, byte toNote);
  float applyAllCC(float baseValue);
};
```

**Avantages :**
- ✓ Vibrato haute qualité (LFO)
- ✓ Portamento fluide
- ✓ Enveloppes ADSR possibles
- ✓ Code organisé

**Inconvénients :**
- ✗ Complexité accrue
- ✗ Plus de CPU
- ✗ Risque de latence

---

### Option 3 : CC selectifs (minimaliste)

Implémenter UNIQUEMENT :
- **CC 7** (Volume)
- **CC 11** (Expression)
- **CC 120** (All Sound Off)

**Avantages :**
- ✓ Très simple
- ✓ Couvre 80% des besoins
- ✓ Performances optimales

**Inconvénients :**
- ✗ Pas d'effets expressifs (vibrato, etc.)
- ✗ Pas de breath controller

---

## 🎯 Recommandations par priorité

### Phase 1 : Essentiels (implémentation rapide)
1. **CC 7 (Volume)** - Multiplicateur global
2. **CC 11 (Expression)** - Contrôle dynamique
3. **CC 120 (All Sound Off)** - Sécurité

**Temps estimé :** Simple, architecture existante
**Impact :** Contrôle volume basique fonctionnel

---

### Phase 2 : Expressivité (si besoin artistique)
4. **CC 1 (Modulation/Vibrato)** - Effet vibrato simple
5. **CC 74 (Brightness)** - Contrôle timbre
6. **CC 64 (Sustain)** - Notes liées

**Temps estimé :** Moyen, ajout logique calcul
**Impact :** Performance musicale enrichie

---

### Phase 3 : Avancé (si hardware breath controller)
7. **CC 2 (Breath Controller)** - Contrôle souffle
8. **CC 5 + 65 + 84 (Portamento)** - Glissements notes

**Temps estimé :** Important, nécessite tests
**Impact :** Jeu très expressif, proche instrument réel

---

## 📋 Questions à trancher avant implémentation

### 1. Niveau de complexité désiré ?
- [ ] Minimaliste (CC 7, 11, 120 seulement)
- [ ] Standard (+ CC 1, 64, 74)
- [ ] Avancé (+ CC 2, portamento)

### 2. Vibrato (CC1) : quelle implémentation ?
- [ ] Option A : Simple offset sinusoïdal (facile)
- [ ] Option B : LFO avancé avec profondeur/vitesse (complexe)
- [ ] Option C : Pas de vibrato

### 3. Breath Controller (CC2) ?
- [ ] Oui, prévu utilisation breath controller hardware
- [ ] Non, velocity suffit
- [ ] À voir plus tard

### 4. Sustain Pedal (CC64) ?
- [ ] Oui, garde valve ouverte entre notes
- [ ] Non, comportement actuel suffit

### 5. Portamento ?
- [ ] Oui, glissements entre notes
- [ ] Non, trop complexe pour servos

### 6. Stockage valeurs CC ?
- [ ] Option A : Variables dans InstrumentManager
- [ ] Option B : Classe CCProcessor dédiée
- [ ] Option C : Variables globales simples

### 7. Compatibilité MIDI ?
- [ ] Supporter tous CC standards (compatibilité max)
- [ ] Seulement CC utiles pour flûte (simplicité)

---

## 🔍 Impact sur configuration actuelle

### Modifications fichiers

**MidiHandler.cpp**
```cpp
case 0xB0:  // Control Change
  byte ccNumber = midiEvent.byte2;
  byte ccValue = midiEvent.byte3;
  _instrument.handleControlChange(ccNumber, ccValue);
  break;
```

**InstrumentManager.h/cpp**
```cpp
void handleControlChange(byte ccNumber, byte ccValue);
byte getCCVolume() const { return _ccVolume; }
byte getCCExpression() const { return _ccExpression; }
// etc.
```

**AirflowController.cpp**
```cpp
// Modifier setAirflowForNote() pour appliquer CC
float volumeMultiplier = _instrument.getCCVolume() / 127.0;
float expressionMultiplier = _instrument.getCCExpression() / 127.0;
angle = baseAngle * volumeMultiplier * expressionMultiplier;
```

**settings.h (optionnel)**
```cpp
// Valeurs par défaut des CC
#define DEFAULT_CC_VOLUME      127
#define DEFAULT_CC_EXPRESSION  127
#define DEFAULT_CC_MODULATION  0
#define DEFAULT_CC_BRIGHTNESS  64
```

---

## 💡 Cas d'usage concrets

### Scénario 1 : Crescendo sur phrase musicale
```
Début phrase : CC11 = 30 (pianissimo)
Milieu phrase : CC11 = 90 (crescendo)
Fin phrase : CC11 = 127 (fortissimo)

→ Notes gardent même velocity, mais airflow augmente dynamiquement
```

### Scénario 2 : Volume global concert
```
Répétition : CC7 = 80 (volume modéré)
Concert : CC7 = 127 (volume max)

→ Ajustement global sans modifier séquence MIDI
```

### Scénario 3 : Expression vibrato
```
Note tenue Do6 :
  CC1 = 40 (vibrato modéré)
  → Airflow oscille ±5° autour de la valeur cible
  → Simule vibrato naturel flûtiste
```

### Scénario 4 : Breath controller
```
Musician souffle dans breath controller USB
  → CC2 envoyé en temps réel (0-127)
  → Remplace velocity
  → Contrôle naturel comme vraie flûte
```

---

## 📊 Tableau récapitulatif

| CC  | Nom             | Priorité | Difficulté | Impact musical | Impact code |
|-----|-----------------|----------|------------|----------------|-------------|
| 7   | Volume          | ⭐⭐⭐    | Facile     | Moyen          | Faible      |
| 11  | Expression      | ⭐⭐⭐    | Facile     | Élevé          | Faible      |
| 120 | All Sound Off   | ⭐⭐⭐    | Facile     | Faible (sécu)  | Faible      |
| 1   | Modulation      | ⭐⭐      | Moyen      | Élevé          | Moyen       |
| 64  | Sustain         | ⭐⭐      | Facile     | Moyen          | Moyen       |
| 74  | Brightness      | ⭐⭐      | Facile     | Moyen          | Faible      |
| 2   | Breath          | ⭐        | Facile     | Très élevé*    | Faible      |
| 5   | Portamento Time | ⭐        | Difficile  | Faible         | Élevé       |

*Si hardware breath controller disponible

---

## 🎬 Proposition de roadmap

### Étape 1 : Volume basique
- Implémenter CC 7 (Volume)
- Implémenter CC 11 (Expression)
- Tester avec DAW (Reaper, Ableton, etc.)

### Étape 2 : Sécurité
- Implémenter CC 120 (All Sound Off)
- Implémenter CC 121 (Reset)

### Étape 3 : Expressivité
- Implémenter CC 1 (Modulation/Vibrato)
- Implémenter CC 74 (Brightness)

### Étape 4 : Avancé (optionnel)
- Implémenter CC 2 (Breath)
- Implémenter CC 64 (Sustain)
- Tester portamento si pertinent

---

## ❓ Prochaines étapes

Avant d'implémenter, décider :

1. **Quels CC implémenter ?** (recommandé : 7, 11, 120 minimum)
2. **Architecture ?** (recommandé : Option 1 - multiplicateurs simples)
3. **Vibrato ?** (simple sinusoïde ou LFO ?)
4. **Breath controller ?** (prévu ou non ?)
5. **Tests ?** (DAW à utiliser pour validation ?)

**Une fois ces choix faits, je peux implémenter le code correspondant.**
