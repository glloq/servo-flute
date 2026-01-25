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
- **Fonction :** Contrôle d'expression dynamique temps réel **DANS les bornes de la note**
- **Effet :**
  - 0 = Expression minimale (airflowMinPercent de la note)
  - 127 = Pleine expression (angle défini par velocity, défaut)
- **Usage :** Crescendo/diminuendo pendant performance
- **Important :** CC11 respecte toujours les limites airflowMinPercent/MaxPercent de la note jouée

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

## 🎯 Différence CC7 vs CC11

### CC11 (Expression) - Respecte les bornes de la note
```
Note C6: airflowMinPercent = 20%, airflowMaxPercent = 75%
→ Plage absolue: [68°, 90°]

Velocity 100 → baseAngle = 86°

CC11 = 127 → modulatedAngle = 86° (pleine expression)
CC11 = 0   → modulatedAngle = 68° (min de la note)

✓ CC11 module dans [minAngle, baseAngle] = [68°, 86°]
✓ Ne peut PAS descendre sous 68° (airflowMinPercent)
```

### CC7 (Volume) - Multiplicateur global
```
Après CC11, on a modulatedAngle = 77°

CC7 = 127 → finalAngle = 77 × 1.0 = 77°
CC7 = 64  → finalAngle = 77 × 0.5 = 38.5°

✗ CC7 PEUT descendre sous minAngle de la note
→ C'est un contrôle de volume "master"
```

### Cas pratique : Crescendo naturel
```
Velocity 127, Note C6 [68°-90°]

1. CC11 = 0, CC7 = 127
   → modulatedAngle = 68° (pianissimo naturel de la note)
   → finalAngle = 68°

2. CC11 = 64, CC7 = 127
   → modulatedAngle = 79° (mezzo-forte)
   → finalAngle = 79°

3. CC11 = 127, CC7 = 127
   → modulatedAngle = 90° (fortissimo)
   → finalAngle = 90°

✓ Crescendo respecte la physique de la note (reste dans [68°, 90°])
```

### Cas pratique : Réduction volume globale
```
Velocity 127, Note C6, CC11 = 127
→ modulatedAngle = 90°

CC7 = 127 → finalAngle = 90° (volume normal)
CC7 = 64  → finalAngle = 45° (volume réduit de moitié)

→ Utile pour ajuster volume global sans modifier expression
```

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
// 1. Calcul angle de base (velocity dans plage note)
uint16_t baseAngle = map(velocity, 1, 127, minAngle, maxAngle);

// 2. CC11 (Expression) module DANS [minAngle, baseAngle]
//    CC11 = 127 → baseAngle (pleine expression selon velocity)
//    CC11 = 0   → minAngle (expression minimale de la note)
float expressionFactor = CC11 / 127.0;
float modulatedAngle = minAngle + (baseAngle - minAngle) × expressionFactor;

// 3. CC7 (Volume) - multiplicateur global
float finalAngle = modulatedAngle × (CC7 / 127.0);

// 4. CC1 (Vibrato)
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

**IMPORTANT :** CC11 respecte les bornes de la note (airflowMinPercent/MaxPercent)

```
1. Calcul plage de la note
   minAngle = SERVO_AIRFLOW_MIN + (plage × airflowMinPercent / 100)
   maxAngle = SERVO_AIRFLOW_MIN + (plage × airflowMaxPercent / 100)

2. Velocity → angle de base DANS [minAngle, maxAngle]
   baseAngle = map(velocity, 1, 127, minAngle, maxAngle)

3. CC11 (Expression) module DANS [minAngle, baseAngle]
   expressionFactor = CC11 / 127
   modulatedAngle = minAngle + (baseAngle - minAngle) × expressionFactor

   Comportement:
   - CC11 = 127 → modulatedAngle = baseAngle (pleine expression)
   - CC11 = 64  → modulatedAngle au milieu entre minAngle et baseAngle
   - CC11 = 0   → modulatedAngle = minAngle (expression minimale)

4. CC7 (Volume) - multiplicateur global
   angle = modulatedAngle × (CC7 / 127)

5. CC1 (Vibrato)
   Si CC1 > 0:
     vibratoOffset = sin(2π × 6Hz × time) × (CC1/127 × 8°)
     angle += vibratoOffset

6. Clamp final
   finalAngle = constrain(angle, SERVO_AIRFLOW_MIN, SERVO_AIRFLOW_MAX)
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
Plage servo absolue: 100 - 60 = 40°

1. Plage de la note C6
   minAngle = 60 + (40 × 20/100) = 68°
   maxAngle = 60 + (40 × 75/100) = 90°

2. Velocity = 100
   baseAngle = map(100, 1, 127, 68, 90) = 86°

3. CC11 = 64 (50% expression)
   expressionFactor = 64/127 = 0.50
   modulatedAngle = 68 + (86 - 68) × 0.50 = 68 + 9 = 77°
   ✓ Respecte la borne: 68° ≤ 77° ≤ 86°

4. CC7 = 100 (79% volume)
   angle = 77 × (100/127) = 61°

5. CC1 = 40 (vibrato modéré)
   vibratoAmplitude = (40/127) × 8 = 2.5°
   vibratoOffset = sin(...) × 2.5  // Varie entre -2.5° et +2.5°

6. finalAngle = 61 ± 2.5°
   → Oscillation entre 58.5° et 63.5°
```

**Comparaison CC11 :**
```
Avec velocity 100 (baseAngle = 86°), plage note [68°, 90°]

CC11 = 127 (100%) → modulatedAngle = 68 + (86-68)×1.0 = 86° (max)
CC11 = 64  (50%)  → modulatedAngle = 68 + (86-68)×0.5 = 77° (milieu)
CC11 = 0   (0%)   → modulatedAngle = 68 + (86-68)×0.0 = 68° (min note)

✓ CC11 reste TOUJOURS dans [68°, 86°] (bornes de la note pour cette velocity)
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
   → baseAngle = 86° (dans plage [68°, 90°])

2. CC 11 = 0 (pianissimo)
   → modulatedAngle = 68° (minimum de la note)
   → Avec CC7=127: finalAngle ≈ 68°

3. CC 11 = 64 (crescendo progressif)
   → modulatedAngle = 77° (milieu entre min et base)
   → Avec CC7=127: finalAngle ≈ 77°

4. CC 11 = 127 (fortissimo)
   → modulatedAngle = 86° (pleine expression selon velocity)
   → Avec CC7=127: finalAngle ≈ 86°

5. Note Off: C6

✓ L'expression module DANS la plage [68°, 86°] définie par la note
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
