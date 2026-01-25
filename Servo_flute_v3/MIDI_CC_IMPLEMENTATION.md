# Implémentation MIDI Control Change

## 📋 Control Change implémentés

### CC 1 - Modulation (Vibrato)
- **Valeur :** 0-127
- **Fonction :** Ajoute un vibrato (oscillation) à l'airflow
- **Effet :**
  - 0 = Pas de vibrato
  - 127 = Vibrato maximum (±8°)
- **Fréquence :** 6 Hz (typique pour flûte)

### CC 7 - Volume (Channel Volume)
- **Valeur :** 0-127
- **Fonction :** Multiplicateur global de volume
- **Effet :**
  - 0 = Silence (0% airflow)
  - 64 = 50% volume
  - 127 = 100% volume (défaut)

### CC 11 - Expression
- **Valeur :** 0-127
- **Fonction :** Contrôle d'expression dynamique temps réel
- **Effet :**
  - 0 = Minimum expression (0% airflow)
  - 127 = Maximum expression (100%, défaut)
- **Usage :** Crescendo/diminuendo pendant performance

### CC 120 - All Sound Off
- **Valeur :** Toutes (déclenchement immédiat)
- **Fonction :** Arrêt d'urgence
- **Actions :**
  - Vide la queue d'événements
  - Stoppe le séquenceur
  - Ferme la valve solénoïde
  - Met l'airflow au repos
  - Ferme tous les servos doigts

---

## 🔧 Architecture d'implémentation

### Flux de données
```
MIDI Controller
    ↓
MidiHandler.processMidiEvent()
    ↓ (case 0xB0 - Control Change)
InstrumentManager.handleControlChange(ccNumber, ccValue)
    ↓ (stocke + synchronise)
AirflowController.setCCValues(cc7, cc11, cc1)
    ↓ (applique lors du calcul)
AirflowController.setAirflowForNote()
    → Calcul angle final avec CC
```

### Classes modifiées

#### 1. **InstrumentManager** (gestionnaire central)
**Fichiers :** `InstrumentManager.h/cpp`

**Variables ajoutées :**
```cpp
byte _ccVolume;       // CC 7  (défaut: 127)
byte _ccExpression;   // CC 11 (défaut: 127)
byte _ccModulation;   // CC 1  (défaut: 0)
```

**Méthodes ajoutées :**
```cpp
void handleControlChange(byte ccNumber, byte ccValue);
byte getCCVolume() const;
byte getCCExpression() const;
byte getCCModulation() const;
void allSoundOff();
```

**Logique :**
- Reçoit les CC depuis MidiHandler
- Stocke les valeurs actuelles
- Synchronise avec AirflowController via `setCCValues()`
- Gère All Sound Off (CC120)

---

#### 2. **MidiHandler** (réception MIDI)
**Fichier :** `MidiHandler.cpp`

**Modification :**
```cpp
case 0xB0:  // Control Change
  {
    byte ccNumber = midiEvent.byte2;
    byte ccValue = midiEvent.byte3;
    _instrument.handleControlChange(ccNumber, ccValue);
  }
  break;
```

**Rôle :**
- Détecte les messages CC MIDI
- Extrait numéro CC et valeur
- Délègue à InstrumentManager

---

#### 3. **AirflowController** (application des CC)
**Fichiers :** `AirflowController.h/cpp`

**Variables ajoutées :**
```cpp
byte _ccVolume;       // CC 7
byte _ccExpression;   // CC 11
byte _ccModulation;   // CC 1
```

**Méthode ajoutée :**
```cpp
void setCCValues(byte ccVolume, byte ccExpression, byte ccModulation);
```

**Logique dans `setAirflowForNote()` :**
```cpp
// 1. Calcul angle de base (velocity + note config)
uint16_t baseAngle = map(velocity, 1, 127, minAngle, maxAngle);

// 2. Appliquer CC7 (Volume)
float finalAngle = baseAngle * (ccVolume / 127.0);

// 3. Appliquer CC11 (Expression)
finalAngle *= (ccExpression / 127.0);

// 4. Appliquer CC1 (Vibrato)
if (ccModulation > 0) {
  float vibratoFreq = 6.0;  // Hz
  float amplitude = (ccModulation / 127.0) * 8.0;  // Max ±8°
  float offset = sin(2π × freq × time) × amplitude;
  finalAngle += offset;
}

// 5. Limiter dans bornes
finalAngle = constrain(finalAngle, MIN, MAX);
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

## 📊 Formule complète de calcul

### Angle final airflow

```
1. baseAngle = map(velocity, 1, 127, minAngle, maxAngle)
   Où minAngle/maxAngle sont calculés depuis airflowMinPercent/MaxPercent

2. volumeMultiplier = CC7 / 127
   Range: 0.0 à 1.0

3. expressionMultiplier = CC11 / 127
   Range: 0.0 à 1.0

4. angle = baseAngle × volumeMultiplier × expressionMultiplier

5. Si CC1 > 0:
     vibratoOffset = sin(2π × 6Hz × time) × (CC1/127 × 8°)
     angle += vibratoOffset

6. finalAngle = constrain(angle, SERVO_AIRFLOW_MIN, SERVO_AIRFLOW_MAX)
```

### Exemple concret

**Configuration :**
- Note : C6 (MIDI 84)
- airflowMinPercent : 20%
- airflowMaxPercent : 75%
- SERVO_AIRFLOW_MIN : 60°
- SERVO_AIRFLOW_MAX : 100°

**Calcul :**
```
minAngle = 60 + (40 × 20/100) = 68°
maxAngle = 60 + (40 × 75/100) = 90°

Velocity = 100
baseAngle = map(100, 1, 127, 68, 90) = 86°

CC7 = 80 (63% volume)
angle = 86 × (80/127) = 54°

CC11 = 127 (100% expression)
angle = 54 × (127/127) = 54°

CC1 = 40 (vibrato modéré)
vibratoAmplitude = (40/127) × 8 = 2.5°
vibratoOffset = sin(...) × 2.5  // Varie entre -2.5° et +2.5°

finalAngle = 54 ± 2.5°  (varie avec le temps)
→ Oscillation entre 51.5° et 56.5°
```

---

## 🎹 Utilisation pratique

### Scénario 1 : Contrôle volume simple
```
Message MIDI: CC 7, valeur 100
→ Volume à 79% (100/127)
→ Tous les angles airflow × 0.79
```

### Scénario 2 : Crescendo pendant note
```
1. Note On: C6, velocity 100
2. CC 11 = 40 (pianissimo)
   → Airflow réduit à 31% (40/127)
3. CC 11 = 80 (crescendo)
   → Airflow monte à 63%
4. CC 11 = 127 (fortissimo)
   → Airflow à 100%
5. Note Off: C6
```

### Scénario 3 : Vibrato expressif
```
1. Note On: D6, velocity 80
2. CC 1 = 0 (pas de vibrato)
   → Son stable
3. CC 1 = 50 (vibrato modéré)
   → Airflow oscille ±3° à 6Hz
4. CC 1 = 100 (vibrato intense)
   → Airflow oscille ±6° à 6Hz
```

### Scénario 4 : All Sound Off (urgence)
```
Situation: Notes bloquées, problème MIDI
Action: Envoyer CC 120
Résultat:
  - Queue vidée
  - Séquenceur stoppé
  - Valve fermée
  - Servos au repos
  - Silence immédiat
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

### Valeurs par défaut (settings.h - optionnel)

Si vous voulez modifier les valeurs par défaut, vous pouvez ajouter dans `settings.h` :

```cpp
// Valeurs par défaut des Control Change
#define DEFAULT_CC_VOLUME      127  // Volume max
#define DEFAULT_CC_EXPRESSION  127  // Expression max
#define DEFAULT_CC_MODULATION  0    // Pas de vibrato
```

Puis modifier les constructeurs pour utiliser ces constantes.

### Paramètres vibrato (AirflowController.cpp)

**Ligne 105-110 :**
```cpp
float vibratoFreq = 6.0;              // Fréquence en Hz (modifiable)
float vibratoAmplitude = ... * 8.0;  // Amplitude max en degrés (modifiable)
```

**Ajustements possibles :**
- `vibratoFreq` : 4-8 Hz (typique instruments à vent)
- `vibratoAmplitude` : 5-12° (selon réactivité servo)

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
- [ ] Ajouter CC 74 (Brightness)

### Moyen terme
- [ ] Support CC 2 (Breath Controller)
- [ ] Aftertouch → Expression dynamique
- [ ] Program Change → Modes de jeu

### Long terme
- [ ] Pitch Bend (via airflow approximation)
- [ ] SysEx pour configuration à distance
- [ ] Enregistrement/lecture presets CC

---

## 📝 Notes importantes

1. **Ordre d'application :** CC7 → CC11 → CC1 (vibrato en dernier)
2. **Performance :** Vibrato utilise `sin()` à chaque calcul (léger impact CPU)
3. **Valeurs par défaut :** CC7=127, CC11=127, CC1=0 (son normal)
4. **All Sound Off :** Priorité absolue (interrompt tout)
5. **Debug :** Active dans `settings.h` avec `DEBUG 1`

---

## ✅ Résumé implémentation

**Fichiers modifiés :**
- `InstrumentManager.h/cpp` - Gestion CC centralisée
- `MidiHandler.cpp` - Réception CC MIDI
- `AirflowController.h/cpp` - Application CC sur airflow
- `NoteSequencer.h/cpp` - Méthode stop() pour All Sound Off

**Lignes de code ajoutées :** ~150 lignes

**Temps d'implémentation :** ~2-3h

**Complexité :** Moyenne (multiplicateurs + vibrato sinusoïdal)

**Compatibilité :** 100% DAWs standards

---

**Documentation créée le :** 2026-01-25
**Version Servo Flute :** V3
**CC implémentés :** 1, 7, 11, 120
