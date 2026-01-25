# Analyse : Autres Messages MIDI Applicables

## 📨 Messages MIDI actuellement marqués "Non implémenté"

D'après `MidiHandler.cpp`, ces messages sont reçus mais ignorés :

1. **0xE0 - Pitch Bend** (Pitch Wheel)
2. **0xA0 - Channel Pressure** (Aftertouch polyphonique)
3. **0xD0 - Polyphonic Key Pressure** (Aftertouch canal)
4. **0xF0 - System Messages**

---

## 🎵 Pitch Bend (0xE0)

### Description MIDI
- **Roue de pitch bend** (molette sur clavier MIDI)
- **Valeur :** 0-16383 (14 bits)
  - Centre : 8192
  - Min : 0 (-2 demi-tons par défaut)
  - Max : 16383 (+2 demi-tons par défaut)
- **Résolution :** Très fine (16384 valeurs)

### Applications pour flûte

#### Option A : Pitch bend via airflow (simulation)
**Concept :** Modifier légèrement l'airflow pour simuler changement hauteur
```
Pitch bend up (+1 demi-ton) → Augmente airflow +5-10%
Pitch bend down (-1 demi-ton) → Réduit airflow -5-10%
```

**Formule :**
```cpp
int pitchBendValue = // 0-16383
int bendOffset = pitchBendValue - 8192; // -8192 à +8192

// Convertir en pourcentage (-10% à +10%)
float bendPercent = (bendOffset / 8192.0) * 0.10;

angleFinal = angleBase * (1.0 + bendPercent);
```

**Réalisme :** ⭐⭐☆☆☆
- Vraie flûte : pitch bend = modification doigtés + embouchure
- Notre système : seulement airflow (approximation)
- Utile pour micro-ajustements, pas pour bends dramatiques

---

#### Option B : Pitch bend via servos doigts (demi-trous)
**Concept :** Ouvrir partiellement un trou pour baisser pitch

**Limitations :**
- ❌ Système actuel = **mode binaire uniquement** (fermé/ouvert)
- ❌ Pas de positions intermédiaires implémentées
- ❌ Nécessiterait refonte complète architecture

**Conclusion :** Non réalisable sans changer spécifications

---

#### Option C : Pitch bend désactivé
**Justification :**
- Flûte = instrument à hauteur fixe par doigté
- Pitch bend peu naturel pour ce type d'instrument
- Complexité vs bénéfice musical faible

**Recommandation :** ⭐⭐⭐ (ignorer pitch bend)

---

## 🎹 Aftertouch (0xA0 - Polyphonic / 0xD0 - Channel)

### Description MIDI

#### Polyphonic Aftertouch (0xA0)
- Pression APRÈS enfoncement touche, **par note**
- Rare sur claviers (coûteux)

#### Channel Aftertouch (0xD0)
- Pression globale sur canal
- Plus courant sur claviers

### Applications pour flûte

#### Option A : Aftertouch → Expression dynamique
**Concept :** Modifier airflow PENDANT qu'une note est tenue

```cpp
void handleAftertouch(byte note, byte pressure) {
  // Modifier airflow note en cours
  float pressureMultiplier = pressure / 127.0;
  updateCurrentNoteAirflow(pressureMultiplier);
}
```

**Cas d'usage :**
```
Note Do6 jouée (velocity 80)
  ↓
Pianiste appuie plus fort sur touche
  ↓
Aftertouch 100 envoyé
  ↓
Airflow augmente dynamiquement (swells)
```

**Réalisme :** ⭐⭐⭐⭐☆
- Simule variation souffle flûtiste
- Très expressif pour notes tenues
- Naturel pour crescendo/diminuendo

---

#### Option B : Aftertouch → Vibrato
**Concept :** Pression touche active/intensifie vibrato

```cpp
vibratoDepth = aftertouch;  // Plus de pression = plus de vibrato
```

**Avantages :**
- ✓ Contrôle naturel vibrato
- ✓ Complémente CC1 (Modulation)

**Inconvénients :**
- ✗ Conflit potentiel si CC1 déjà utilisé pour vibrato

---

#### Option C : Aftertouch → Brightness
**Concept :** Pression touche change timbre (comme CC74)

```cpp
brightness = 0.7 + (aftertouch / 127.0) * 0.6;
```

---

### Recommandations Aftertouch

**Si clavier supporte Aftertouch :**
- ⭐⭐⭐ Implémenter comme **Expression dynamique** (Option A)
- Alternative : Vibrato ou Brightness

**Si pas d'Aftertouch :**
- Ignorer, utiliser CC11 (Expression) à la place

---

## 🔧 System Messages (0xF0)

### Messages pertinents

#### System Exclusive (SysEx) - 0xF0
**Usage :** Configuration custom, dumps, presets

**Applications possibles :**
- Charger/sauvegarder configurations instrument (settings.h)
- Calibration servos à distance
- Changer instrument (flûte soprano ↔ Irish flute)

**Complexité :** ⭐⭐⭐⭐⭐ (Très élevée)

**Recommandation :** ❌ Pas prioritaire (config via USB/Serial suffit)

---

#### MIDI Clock - 0xF8
**Usage :** Synchronisation tempo

**Applications :**
- Synchroniser vibrato avec tempo
- Synchroniser effets rythmiques

**Recommandation :** ⭐☆☆☆☆ (peu utile pour flûte)

---

#### Active Sensing - 0xFE
**Usage :** Détection connexion MIDI active

**Applications :**
- Sécurité : fermer valve si connexion perdue
- Évite notes bloquées

**Recommandation :** ⭐⭐☆☆☆ (nice-to-have, pas essentiel)

---

#### System Reset - 0xFF
**Usage :** Reset complet système MIDI

**Applications :**
- Équivalent CC 120 (All Sound Off)
- Position repos tous servos
- Reset tous CC à défaut

**Recommandation :** ⭐⭐⭐ (sécurité)

---

## 🎛️ Program Change (0xC0) - Non mentionné mais utile !

### Description
- **Message :** 0xC0 + program number (0-127)
- **Usage MIDI :** Changer patch/instrument/preset

### Applications pour Servo Flute

#### Option A : Changer instrument
**Concept :** Basculer entre configurations

```cpp
void handleProgramChange(byte program) {
  switch(program) {
    case 0:  loadRecorderSettings();      break;  // Flûte à bec
    case 1:  loadIrishFluteSettings();    break;  // Irish flute
    case 2:  loadTinWhistleSettings();    break;  // Tin whistle
    // ...
  }
}
```

**Limitations :**
- ❌ Nécessite configurations en RAM (mémoire limitée Arduino)
- ❌ settings.h actuellement compilé (pas dynamique)

**Alternative :**
- Presets = variations d'une même config
  - Program 0 : Airflow normal
  - Program 1 : Airflow -20% (son doux)
  - Program 2 : Airflow +20% (son brillant)

---

#### Option B : Modes de jeu
**Concept :** Comportements différents

```cpp
Program 0 : Mode normal
Program 1 : Mode legato (valve reste ouverte)
Program 2 : Mode staccato (notes courtes)
Program 3 : Mode vibrato automatique
```

---

#### Option C : Octave shift
**Concept :** Transposer toutes les notes

```cpp
Program 0 : Octave normale
Program 1 : Octave +12 (transpose vers aigu)
Program 2 : Octave -12 (transpose vers grave)
```

**Utile pour :** Étendre gamme sans modifier séquence MIDI

---

### Recommandation Program Change

**Priorité :** ⭐⭐☆☆☆ (intéressant mais pas essentiel)

**Usage recommandé :** Modes de jeu ou presets airflow

---

## 📊 Tableau récapitulatif

| Message         | Code | Priorité | Difficulté | Utilité flûte | Recommandation          |
|-----------------|------|----------|------------|---------------|-------------------------|
| Pitch Bend      | 0xE0 | ⭐        | Moyen      | Faible        | ❌ Ignorer              |
| Aftertouch Poly | 0xA0 | ⭐⭐      | Facile     | Élevée*       | ✅ Si clavier supporte  |
| Aftertouch Ch.  | 0xD0 | ⭐⭐      | Facile     | Élevée*       | ✅ Si clavier supporte  |
| Program Change  | 0xC0 | ⭐⭐      | Moyen      | Moyenne       | ⚠️ Modes/presets        |
| System Reset    | 0xFF | ⭐⭐⭐    | Facile     | Moyenne       | ✅ Sécurité             |
| SysEx           | 0xF0 | ⭐        | Très diff. | Faible        | ❌ Pas prioritaire      |
| MIDI Clock      | 0xF8 | ⭐        | Moyen      | Faible        | ❌ Ignorer              |

*Si le clavier MIDI utilisé supporte Aftertouch

---

## 🎯 Recommandations finales

### Messages à implémenter

#### Priorité 1 (Essentiels)
- ✅ **Note On/Off** (déjà fait)
- ✅ **CC 7, 11, 120** (volume, expression, all sound off)
- ✅ **System Reset 0xFF** (sécurité)

#### Priorité 2 (Si clavier compatible)
- ⚠️ **Aftertouch** (expression dynamique notes tenues)
- ⚠️ **CC 1** (vibrato/modulation)

#### Priorité 3 (Nice-to-have)
- ⚠️ **Program Change** (modes de jeu)
- ⚠️ **CC 64** (sustain)
- ⚠️ **CC 74** (brightness)

#### Ignorer
- ❌ **Pitch Bend** (peu naturel pour flûte binaire)
- ❌ **SysEx** (trop complexe)
- ❌ **MIDI Clock** (inutile)

---

## 🔄 Interactions entre messages

### Scénario 1 : Performance expressive complète
```
1. Program Change 0 → Mode normal
2. Note On Do6, velocity 100
3. CC 11 = 60 → Expression modérée
4. Aftertouch 80 → Crescendo pendant note
5. CC 1 = 40 → Ajoute vibrato
6. Note Off Do6
```

**Résultat :**
- Note démarre volume moyen (CC11=60)
- Volume augmente pendant note (Aftertouch 80)
- Vibrato s'ajoute (CC1=40)
- Note s'arrête proprement

---

### Scénario 2 : Gestion erreur
```
1. Notes jouent normalement
2. Connexion MIDI perdue
3. Active Sensing timeout (si implémenté)
   OU
   System Reset 0xFF reçu
4. → All Sound Off immédiat
5. → Valve ferme, servos repos
```

---

### Scénario 3 : Changement style jeu
```
1. Program Change 1 → Mode legato
2. Notes jouent avec valve toujours ouverte
3. Program Change 2 → Mode staccato
4. Notes courtes, valve ferme vite
```

---

## 💡 Architecture proposée (si implémentation complète)

```cpp
class MidiHandler {
  void processMidiEvent(midiEventPacket_t event) {
    byte type = event.byte1 & 0xF0;

    switch(type) {
      case 0x90: handleNoteOn(); break;
      case 0x80: handleNoteOff(); break;
      case 0xB0: handleControlChange(); break;  // CC
      case 0xC0: handleProgramChange(); break;  // NEW
      case 0xE0: handlePitchBend(); break;      // NEW (optionnel)
      case 0xA0: handlePolyAftertouch(); break; // NEW (optionnel)
      case 0xD0: handleChannelAftertouch(); break; // NEW (optionnel)
      case 0xF0: handleSystemMessage(); break;  // NEW
    }
  }
};
```

---

## ❓ Questions avant implémentation

### 1. Aftertouch ?
- [ ] Clavier supporte Aftertouch → implémenter
- [ ] Pas d'Aftertouch → ignorer

### 2. Aftertouch → quelle fonction ?
- [ ] Option A : Expression dynamique (recommandé)
- [ ] Option B : Vibrato
- [ ] Option C : Brightness

### 3. Program Change ?
- [ ] Oui, pour modes de jeu (legato/staccato/normal)
- [ ] Oui, pour presets airflow (-20%/normal/+20%)
- [ ] Non, pas nécessaire

### 4. Pitch Bend ?
- [ ] Oui, via airflow (approximation)
- [ ] Non, ignorer (recommandé)

### 5. System Reset ?
- [ ] Oui, sécurité importante
- [ ] Non, CC 120 suffit

---

## 🎬 Proposition implémentation par phases

### Phase 1 : Foundation (CC basiques)
- CC 7 (Volume)
- CC 11 (Expression)
- CC 120 (All Sound Off)

### Phase 2 : Expressivité
- CC 1 (Modulation/Vibrato)
- CC 74 (Brightness)
- Aftertouch → Expression (si clavier compatible)

### Phase 3 : Avancé
- Program Change → Modes de jeu
- System Reset 0xFF
- CC 64 (Sustain)

### Phase 4 : Optionnel
- Pitch Bend (si vraiment demandé)
- CC 2 (Breath controller)
- SysEx (configuration à distance)

---

**Prêt à implémenter selon vos choix !**
