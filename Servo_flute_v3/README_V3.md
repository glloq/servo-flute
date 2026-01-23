# Servo Flute V3 - Architecture améliorée

## Vue d'ensemble

Version 3 du projet servo-flute avec architecture repensée pour :
- **Simplification** : Mode binaire uniquement (ouvert/fermé)
- **Contrôle d'air hybride** : Servo débit + solénoïde valve
- **Timing précis** : Respect des délais relatifs entre notes MIDI
- **Non-bloquant** : State machine asynchrone pour réactivité maximale
- **Adaptabilité** : Configuration centralisée pour changement d'instrument

## Nouveautés V3

### 1. Mode binaire (vs V2 avec demi-ouvert)
- États des trous : **0 = fermé**, **1 = ouvert** uniquement
- Simplification du code et de la mécanique
- Tableau de doigtés réduit à 21 notes (au lieu de 30)
- Élimination de la complexité des positions intermédiaires

### 2. Contrôle d'air hybride

**Servo de débit** (contrôle analogique)
- Module la quantité d'air selon la vélocité MIDI (1-127)
- Mapping linéaire : velocity → angle servo (60° à 100°)
- Permet nuances pianissimo/fortissimo

**Solénoïde valve** (contrôle on/off)
- Bloque/autorise totalement le flux d'air
- Coupure ultra-rapide (~5-10ms vs ~50-100ms pour servo)
- Activation synchronisée avec doigtés

### 3. EventQueue avec timestamps relatifs

**Problème résolu** : Respecter le timing entre notes MIDI
- Example : Note A (t=0ms), Note B (t=50ms), NoteOff A (t=120ms)
- La V3 rejoue ces événements avec les délais exacts
- Queue FIFO circulaire de 16 événements
- Timestamp relatif au premier événement

### 4. State Machine non-bloquante

**Séquence d'une note** :
```
IDLE → POSITIONING (100ms) → WAITING_STABLE (5ms) → PLAYING → STOPPING → IDLE
```

**Timing** :
- `t=0ms` : Commande servos doigts
- `t=0-100ms` : Déplacement mécanique servos
- `t=100ms` : Servo débit positionné
- `t=105ms` : Solénoïde activé → **SON PRODUIT**

**Avantages** :
- Loop() jamais bloquée
- MIDI lu en continu
- Power management actif
- Pas de `delay()` bloquant

## Architecture technique

### Composants

```
MidiHandler
   ↓ (événements MIDI)
InstrumentManager
   ↓ (coordonne)
   ├─ EventQueue         (buffer événements avec timestamps)
   ├─ NoteSequencer      (state machine)
   ├─ FingerController   (10 servos doigts)
   └─ AirflowController  (servo débit + solénoïde)
       ↓
   PCA9685 PWM Driver + GPIO solénoïde
```

### Fichiers

| Fichier | Rôle |
|---------|------|
| `Servo_flute_v3.ino` | Programme principal Arduino |
| `settings.h` | Configuration centralisée |
| `EventQueue.h/cpp` | File FIFO avec timestamps |
| `NoteSequencer.h/cpp` | State machine de séquencement |
| `FingerController.h/cpp` | Contrôle servos doigts |
| `AirflowController.h/cpp` | Servo + solénoïde |
| `InstrumentManager.h/cpp` | Orchestrateur principal |
| `MidiHandler.h/cpp` | Réception MIDI USB |

## Configuration matérielle

### Servos (PCA9685)
- Canaux 0-9 : Servos doigts
- Canal 10 : Servo débit air

### Solénoïde
- GPIO Pin 13 (configurable dans `settings.h`)
- Transistor/MOSFET requis (200-500mA)
- Alimentation séparée recommandée

### Arduino
- Leonardo ou Micro (USB MIDI natif)
- Pin 5 : OE du PCA9685 (power management)

## Paramètres configurables (`settings.h`)

### Timing
```cpp
#define SERVO_SETTLE_TIME_MS    100   // Délai déplacement servos doigts
#define STABILIZATION_TIME_MS   5     // Délai avant ouverture solénoïde
#define EVENT_QUEUE_SIZE        16    // Taille buffer événements
```

### Servos doigts
```cpp
#define NUMBER_SERVOS_FINGER    10    // Nombre de servos
#define NUMBER_NOTES            21    // Notes jouables
#define ANGLE_OPEN              30    // Angle ouverture (degrés)

const uint16_t closedAngles[10] = {90, 100, 95, ...};  // Calibration
const int sensRotation[10] = {-1, 1, 1, ...};          // Direction
```

### Servo débit + Solénoïde
```cpp
#define NUM_SERVO_AIRFLOW       10    // Canal PCA9685
#define SERVO_AIRFLOW_MIN       60    // Angle velocity=1
#define SERVO_AIRFLOW_MAX       100   // Angle velocity=127

#define SOLENOID_PIN            13    // GPIO solénoïde
#define SOLENOID_ACTIVE_HIGH    true  // Logique HIGH/LOW
```

## Adaptation à d'autres instruments

### Flûte irlandaise 6 trous (exemple)

**Modifications nécessaires** :
1. Dans `settings.h` :
   ```cpp
   #define NUMBER_SERVOS_FINGER 6
   #define NUMBER_NOTES 25  // Plage typique tin whistle
   #define FIRST_MIDI_NOTE 74  // D5 pour tin whistle en D

   // Ajuster calibration pour 6 servos
   const uint16_t closedAngles[6] = {85, 90, 95, 90, 88, 92};
   const int sensRotation[6] = {1, 1, -1, -1, 1, 1};
   ```

2. Refaire le tableau `finger_position[][6]` avec doigtés tin whistle

3. Ajuster `SERVO_SETTLE_TIME_MS` si nécessaire (instrument plus léger)

4. Recompiler et flasher

**Temps estimé** : 1-2h de configuration + calibration

## Utilisation

1. **Flasher** l'Arduino avec l'IDE Arduino
2. **Connecter** via USB (MIDI + Serial debug)
3. **Envoyer** notes MIDI (logiciel ou clavier MIDI)
4. **Observer** logs debug dans Serial Monitor (115200 bauds)

### Debug

Activer/désactiver dans `settings.h` :
```cpp
#define DEBUG 1  // 1=activé, 0=désactivé
```

Messages typiques :
```
DEBUG: FingerController - Pattern appliqué: 0 0 0 0 0 0 0 0 0 0
DEBUG: AirflowController - Vélocité: 80 -> Angle: 75
DEBUG: NoteSequencer - Note jouée: 72 (vélocité: 80)
DEBUG: AirflowController - Solénoïde OUVERT
```

## Limites connues

- **Monophonique** : Une seule note à la fois
- **Latence** : 105ms entre réception MIDI et son produit (délai mécanique)
- **Notes rapides** : Notes < 105ms peuvent être ignorées (ajustable via `MIN_NOTE_DURATION_MS`)
- **Queue limitée** : 16 événements max en buffer

## Comparaison V2 → V3

| Aspect | V2 | V3 |
|--------|----|----|
| États trous | 3 (fermé/ouvert/demi) | 2 (fermé/ouvert) |
| Contrôle air | Servo seul | Servo + solénoïde |
| Timing MIDI | Approximatif | Exact (queue timestamps) |
| Architecture | Bloquante (delays) | Non-bloquante (state machine) |
| Notes jouables | 30 | 21 (extensible) |
| Adaptabilité | Moyenne | Bonne |
| Complexité code | Moyenne | Moyenne-haute |

## Prochaines évolutions possibles

- [ ] Délai adaptatif selon distance angulaire
- [ ] Calibration automatique des servos
- [ ] Support pitch bend
- [ ] Mode polyphonique (arpèges rapides)
- [ ] Stockage profils EEPROM/SD
- [ ] Interface de configuration (LCD + boutons)

## License

Projet open-source - Voir LICENSE dans le dossier racine
