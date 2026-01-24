# GUIDE DE CONFIGURATION - settings.h

## Vue d'ensemble

Le fichier `settings.h` centralise **toute** la configuration du système servo-flute. Ce guide explique comment le modifier pour :
- Adapter à un autre instrument (tin whistle, flûte traversière, etc.)
- Configurer le câblage des servos
- Ajuster les paramètres de timing
- Calibrer les servos et l'airflow

---

## 📋 Structure du fichier

```
settings.h
├─ Configuration instrument (nombre de doigts, notes)
├─ Timing (délais servos, valve)
├─ Solénoïde (PWM, pins)
├─ Servo débit air (angles)
├─ Power management
├─ MAPPING SERVOS → PCA9685  ⭐ Nouveau
├─ Calibration servos doigts
└─ TABLE DES NOTES           ⭐ Nouveau format
```

---

## 🎯 SECTION 1 : Configuration instrument

```cpp
#define NUMBER_SERVOS_FINGER 10  // Nombre de servos doigts
#define NUMBER_NOTES 21          // Nombre de notes jouables
```

### Pour une flûte irlandaise 6 trous :
```cpp
#define NUMBER_SERVOS_FINGER 6
#define NUMBER_NOTES 25
```

⚠️ **Important** : Après modification, ajuster les sections suivantes !

---

## 🔌 SECTION 2 : Mapping servos → PCA9685

### Concept

Le **mapping** sépare l'ordre logique des doigts de l'ordre physique de câblage.

```cpp
const int fingerToPCAChannel[NUMBER_SERVOS_FINGER] = {
  0,  // Doigt 0 (1er trou) → PCA9685 canal 0
  1,  // Doigt 1 (2e trou)  → PCA9685 canal 1
  2,  // Doigt 2 (3e trou)  → PCA9685 canal 2
  // ...
};
```

### Exemple : Ordre inversé

Si vous avez câblé les servos à l'envers :
```cpp
const int fingerToPCAChannel[10] = {
  9, 8, 7, 6, 5, 4, 3, 2, 1, 0  // Ordre inversé
};
```

### Exemple : Câblage personnalisé

```cpp
// Flûte à bec : ordre physique spécifique
const int fingerToPCAChannel[10] = {
  2,  // Doigt 0 → Canal 2
  5,  // Doigt 1 → Canal 5
  0,  // Doigt 2 → Canal 0
  1,  // Doigt 3 → Canal 1
  3,  // Doigt 4 → Canal 3
  4,  // Doigt 5 → Canal 4
  6,  // Doigt 6 → Canal 6
  7,  // Doigt 7 → Canal 7
  8,  // Doigt 8 → Canal 8
  9   // Doigt 9 → Canal 9
};
```

**Avantage** : Modifier le câblage sans changer les doigtés !

---

## 🎵 SECTION 3 : Table des notes

### Format de la structure

```cpp
struct NoteDefinition {
  byte midiNote;                            // Numéro MIDI (72-127)
  bool fingerPattern[NUMBER_SERVOS_FINGER]; // Doigtés (0=fermé, 1=ouvert)
  byte airflowMinPercent;                   // % min servo flow (0-100)
  byte airflowMaxPercent;                   // % max servo flow (0-100)
};
```

### Exemple complet

```cpp
const NoteDefinition NOTES[NUMBER_NOTES] = {
  // MIDI  Doigtés                        Min%  Max%
  {  72,  {0,0,0,0,0,0,0,0,0,0},  0,   50  },  // Do5 grave
  {  73,  {0,0,0,0,0,0,0,0,0,1},  0,   50  },  // Do#5
  {  74,  {0,0,0,0,0,0,0,0,1,1},  0,   50  },  // Ré5
  // ...
};
```

### Champs détaillés

#### 1. `midiNote` (byte)
- Numéro MIDI standard (0-127)
- Flûte à bec soprano : 72-92 (Do5-Sol#6)
- Tin whistle D : 74-98 (Ré5-Ré7)

#### 2. `fingerPattern[10]` (bool[])
- **0 ou false** = trou fermé
- **1 ou true** = trou ouvert
- Index 0 = premier trou, 9 = dernier trou
- ⚠️ Taille doit correspondre à `NUMBER_SERVOS_FINGER`

#### 3. `airflowMinPercent` (byte 0-100)
- Pourcentage MINIMUM d'ouverture du servo débit pour cette note
- Appliqué sur la plage [SERVO_AIRFLOW_MIN, SERVO_AIRFLOW_MAX]
- Vélocité MIDI 1 → angle = min + (max-min) × airflowMinPercent / 100

#### 4. `airflowMaxPercent` (byte 0-100)
- Pourcentage MAXIMUM d'ouverture du servo débit pour cette note
- Appliqué sur la plage [SERVO_AIRFLOW_MIN, SERVO_AIRFLOW_MAX]
- Vélocité MIDI 127 → angle = min + (max-min) × airflowMaxPercent / 100

**Cas d'usage** :
- Notes graves (Do5-Si5) : 0%-50% (moins de pression)
- Notes médium (Do6-Si6) : 20%-80% (pression moyenne)
- Notes aiguës (Do7+) : 40%-100% (forte pression)
- Permet gestion du **volume** (vélocité) ET des **octaves** (plage %)

### Exemple : Tin whistle 6 trous

```cpp
#define NUMBER_SERVOS_FINGER 6
#define NUMBER_NOTES 25

const NoteDefinition NOTES[NUMBER_NOTES] = {
  // MIDI  Doigtés (6 trous)      Min%  Max%
  {  74,  {0,0,0,0,0,0},  0,   50  },  // Ré5 grave - Tous fermés
  {  76,  {0,0,0,0,0,1},  0,   50  },  // Mi5
  {  77,  {0,0,0,0,1,1},  0,   50  },  // Fa#5
  {  78,  {0,0,0,1,1,1},  0,   60  },  // Sol5
  {  79,  {0,0,1,1,1,1},  0,   60  },  // La5
  {  81,  {0,1,1,1,1,1},  0,   60  },  // Si5
  {  83,  {1,1,1,1,1,1},  0,   70  },  // Do#6
  {  86,  {0,0,0,0,0,1},  30,  80  },  // Ré6 - Octave sup, +air
  {  88,  {0,0,0,0,1,1},  30,  80  },  // Mi6
  {  90,  {0,0,0,1,1,1},  30,  90  },  // Fa#6
  {  91,  {0,0,1,1,1,1},  40,  90  },  // Sol6
  {  93,  {0,1,1,1,1,1},  40,  90  },  // La6
  {  95,  {1,1,1,1,1,1},  40,  100 },  // Si6
  // ... autres notes
};
```

---

## 🔧 SECTION 4 : Calibration servos doigts

### Angles fermés

```cpp
const uint16_t closedAngles[NUMBER_SERVOS_FINGER] = {
  90,   // Doigt 0
  100,  // Doigt 1
  95,   // Doigt 2
  // ...
};
```

**Procédure de calibration** :
1. Positionner le bras de servo à 90° sur l'axe
2. Ajuster mécaniquement pour fermer le trou
3. Si pas aligné : modifier l'angle (ex: 85° ou 95°)
4. Répéter pour chaque servo

### Angle d'ouverture

```cpp
#define ANGLE_OPEN 30  // Degrés
```

- Petit instrument (tin whistle) : 20-25°
- Grand instrument (flûte traversière) : 35-45°

### Sens de rotation

```cpp
const int sensRotation[NUMBER_SERVOS_FINGER] = {
  -1,  // Doigt 0 : rotation anti-horaire pour ouvrir
   1,  // Doigt 1 : rotation horaire pour ouvrir
  // ...
};
```

- `1` = Horaire (angle augmente pour ouvrir)
- `-1` = Anti-horaire (angle diminue pour ouvrir)

---

## 🌬️ SECTION 5 : Servo débit air

### Configuration de base

```cpp
#define NUM_SERVO_AIRFLOW 10      // Canal PCA9685
#define SERVO_AIRFLOW_OFF 20      // Angle repos
#define SERVO_AIRFLOW_MIN 60      // Angle pianissimo (velocity=1)
#define SERVO_AIRFLOW_MAX 100     // Angle fortissimo (velocity=127)
```

### Mapping vélocité → Angle (système de pourcentages)

Chaque note définit sa propre plage d'airflow via **airflowMinPercent** et **airflowMaxPercent**.

**Formule** :
```
minAngle = SERVO_AIRFLOW_MIN + ((SERVO_AIRFLOW_MAX - SERVO_AIRFLOW_MIN) × airflowMinPercent / 100)
maxAngle = SERVO_AIRFLOW_MIN + ((SERVO_AIRFLOW_MAX - SERVO_AIRFLOW_MIN) × airflowMaxPercent / 100)

Velocity 1   → minAngle
Velocity 64  → interpolation linéaire
Velocity 127 → maxAngle
```

**Exemple** : Note grave (Do5)
```cpp
{72, {0,0,0,0,0,0,0,0,0,0}, 0, 50}  // 0%-50% de la plage
```
Si SERVO_AIRFLOW_MIN=60° et SERVO_AIRFLOW_MAX=100° :
- minAngle = 60 + (40 × 0/100) = 60°
- maxAngle = 60 + (40 × 50/100) = 80°
- Velocity 1 → 60°, Velocity 127 → 80°

**Exemple** : Note aiguë (Sol6)
```cpp
{91, {1,1,0,1,1,1,1,1,1,1}, 40, 100}  // 40%-100% de la plage
```
- minAngle = 60 + (40 × 40/100) = 76°
- maxAngle = 60 + (40 × 100/100) = 100°
- Velocity 1 → 76°, Velocity 127 → 100°

**Avantages** :
- Gestion du volume (vélocité) pour chaque note
- Adaptation automatique selon octave (graves=moins d'air, aigus=plus d'air)
- Configuration intuitive en pourcentages

---

## ⚡ SECTION 6 : Solénoïde

### Pin et logique

```cpp
#define SOLENOID_PIN 13           // Pin PWM Arduino
#define SOLENOID_ACTIVE_HIGH true // HIGH=activé, LOW=désactivé
```

### Mode PWM (réduction chaleur)

```cpp
#define SOLENOID_USE_PWM true     // Activer PWM
#define SOLENOID_PWM_ACTIVATION 255    // 100% pendant 50ms
#define SOLENOID_PWM_HOLDING    128    // 50% pour maintien
#define SOLENOID_ACTIVATION_TIME_MS 50 // Durée activation
```

**Désactiver PWM** (mode GPIO simple) :
```cpp
#define SOLENOID_USE_PWM false
```

---

## ⏱️ SECTION 7 : Timing

```cpp
#define SERVO_TO_SOLENOID_DELAY_MS  105   // Délai total servos → valve
#define MIN_NOTE_INTERVAL_FOR_VALVE_CLOSE_MS  50  // Seuil valve ouverte
```

### Ajuster le délai servos

- Servos rapides : `90-100ms`
- Servos lents : `120-150ms`
- **Observer l'erreur dans debug** pour ajuster

### Seuil valve intelligente

- **30ms** : Valve ferme souvent (moins d'économie)
- **50ms** : Équilibré (recommandé)
- **100ms** : Valve reste ouverte longtemps (max économie)

---

## 📊 Exemple complet : Tin whistle D (6 trous)

```cpp
// ===== INSTRUMENT =====
#define NUMBER_SERVOS_FINGER 6
#define NUMBER_NOTES 19

// ===== MAPPING PCA9685 =====
const int fingerToPCAChannel[6] = {0, 1, 2, 3, 4, 5};

// ===== CALIBRATION SERVOS =====
#define ANGLE_OPEN 25  // Tin whistle = trous plus petits

const uint16_t closedAngles[6] = {
  88, 92, 90, 93, 89, 91
};

const int sensRotation[6] = {
  1, -1, 1, 1, -1, 1
};

// ===== NOTES =====
const NoteDefinition NOTES[19] = {
  // MIDI  Doigtés (6)        Min%  Max%
  {  74,  {0,0,0,0,0,0},  0,   50  },  // Ré5 grave
  {  76,  {0,0,0,0,0,1},  0,   50  },  // Mi5
  {  77,  {0,0,0,0,1,1},  0,   50  },  // Fa#5
  {  78,  {0,0,0,1,1,1},  0,   60  },  // Sol5
  {  79,  {0,0,1,1,1,1},  0,   60  },  // La5
  {  81,  {0,1,1,1,1,1},  0,   60  },  // Si5
  {  83,  {1,1,1,1,1,1},  0,   70  },  // Do#6
  {  86,  {0,0,0,0,0,1},  30,  80  },  // Ré6 octave 2 - +air
  {  88,  {0,0,0,0,1,1},  30,  80  },  // Mi6
  {  90,  {0,0,0,1,1,1},  30,  90  },  // Fa#6
  {  91,  {0,0,1,1,1,1},  40,  90  },  // Sol6
  {  93,  {0,1,1,1,1,1},  40,  90  },  // La6
  {  95,  {1,1,1,1,1,1},  40,  100 },  // Si6
  {  98,  {0,0,0,0,0,1},  50,  100 },  // Ré7 très aigu - ++air
  {  100, {0,0,0,0,1,1},  50,  100 },  // Mi7
  {  102, {0,0,0,1,1,1},  60,  100 },  // Fa#7
  {  103, {0,0,1,1,1,1},  60,  100 },  // Sol7
  {  105, {0,1,1,1,1,1},  70,  100 },  // La7
  {  107, {1,1,1,1,1,1},  70,  100 }   // Si7
};
```

---

## 🔍 Outils de diagnostic

### Fonctions utilitaires

Le fichier `settings.h` fournit :

```cpp
getNoteByMidi(midiNote)  // Retourne NoteDefinition* ou nullptr
getNoteIndex(midiNote)   // Retourne index dans NOTES[] ou -1
```

### Messages de debug

Avec `DEBUG = 1`, vérifier :
```
DEBUG: FingerController - Note MIDI: 72
DEBUG: AirflowController - Note MIDI: 72 | Vel: 80 | Range: 0%-50% (60°-80°) | Angle: 75
```

- **Note MIDI** : Vérifie que la note existe dans NOTES[]
- **Range** : Affiche la plage de pourcentages configurée
- **Angles** : Affiche minAngle-maxAngle calculés à partir des %
- **Angle** : Angle final du servo débit (mapping de vélocité)

---

## ✅ Checklist après modification

- [ ] `NUMBER_SERVOS_FINGER` = nombre de servos réels
- [ ] `NUMBER_NOTES` = nombre de lignes dans `NOTES[]`
- [ ] `fingerToPCAChannel[]` = taille correcte et valeurs 0-15
- [ ] `closedAngles[]` = taille correcte
- [ ] `sensRotation[]` = taille correcte
- [ ] `NOTES[].fingerPattern[]` = taille correcte (même que `NUMBER_SERVOS_FINGER`)
- [ ] Toutes les notes MIDI sont uniques dans `NOTES[]`
- [ ] Test compilation Arduino ✅
- [ ] Test avec une note (observe debug) ✅
- [ ] Calibration fine si nécessaire ✅

---

## 🚀 Workflow de configuration

1. **Définir l'instrument**
   ```cpp
   #define NUMBER_SERVOS_FINGER 6
   #define NUMBER_NOTES 19
   ```

2. **Câbler les servos**
   - Noter l'ordre physique de branchement
   - Remplir `fingerToPCAChannel[]`

3. **Calibrer mécaniquement**
   - Ajuster `closedAngles[]`
   - Tester ouverture/fermeture
   - Ajuster `sensRotation[]` si inversé

4. **Définir les notes**
   - Remplir `NOTES[]` avec doigtés corrects
   - Commencer avec `minAirflow = 0`

5. **Ajuster airflow**
   - Tester notes graves/aiguës
   - Si note mal sonnée : augmenter `minAirflow`

6. **Optimiser timing**
   - Observer logs debug "Erreur:"
   - Ajuster `SERVO_TO_SOLENOID_DELAY_MS`

---

## 📚 Ressources

- [README_V3.md](README_V3.md) - Vue d'ensemble système
- [TIMING_ANTICIPATION.md](TIMING_ANTICIPATION.md) - Détails timing
- [VALVE_OPTIMIZATION.md](VALVE_OPTIMIZATION.md) - Optimisation valve
- [SOLENOID_PWM.md](SOLENOID_PWM.md) - Mode PWM solénoïde

---

**Le fichier `settings.h` est maintenant le SEUL fichier à modifier pour toute configuration d'instrument !** 🎵
