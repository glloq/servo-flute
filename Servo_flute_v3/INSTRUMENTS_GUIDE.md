# Guide de Configuration Multi-Instruments

## Vue d'ensemble

Le système Servo Flute V3 supporte plusieurs types d'instruments grâce à une architecture modulaire basée sur un fichier de configuration centralisé.

## Instruments Disponibles

### 1. **Flûte à bec soprano** (settings.h)
- **Fichier:** `settings.h`
- **Trous:** 10 servos doigts
- **Tonalité:** Do (C)
- **Gamme:** Do5 (C5) à Sol#6 (G#6)
- **Notes MIDI:** 72 à 92
- **Nombre de notes:** 21

**Utilisation:**
```cpp
// Déjà par défaut dans Servo_flute_v3.ino
#include "settings.h"
```

---

### 2. **Flûte irlandaise en D** (settings_irish_flute.h)
- **Fichier:** `settings_irish_flute.h`
- **Trous:** 6 servos doigts
- **Tonalité:** Ré (D majeur)
- **Gamme:** Ré5 (D5) à Ré7 (D7)
- **Notes MIDI:** 74 à 98
- **Nombre de notes:** 15

**Caractéristiques:**
- Doigtés standard Irish flute / Tin whistle
- 3 octaves supportées (grave, médium, aigu)
- Airflow adapté par octave :
  - Octave 1 : 0-70% (faible pression)
  - Octave 2 : 30-95% (pression moyenne)
  - Octave 3 : 50-100% (forte pression)

**Utilisation:**
```cpp
// Dans Servo_flute_v3.ino, modifier la ligne 27 :
//#include "settings.h"              // Commenter l'ancienne ligne
#include "settings_irish_flute.h"    // Activer cette ligne
```

---

## Comment changer d'instrument

### Méthode 1: Modification directe

1. Ouvrir `Servo_flute_v3.ino`
2. Localiser la ligne `#include "settings.h"`
3. Commenter/décommenter selon l'instrument désiré:

```cpp
// Pour flûte à bec soprano (10 trous):
#include "settings.h"

// Pour flûte irlandaise (6 trous):
//#include "settings_irish_flute.h"
```

### Méthode 2: Copier-coller le contenu

Vous pouvez aussi copier le contenu du fichier instrument désiré dans `settings.h` pour le rendre permanent.

---

## Configuration détaillée Irish Flute

### Mapping des trous
```
Trou 0 (haut) -----> Main gauche - Index
Trou 1       -----> Main gauche - Majeur
Trou 2       -----> Main gauche - Annulaire
Trou 3       -----> Main droite - Index
Trou 4       -----> Main droite - Majeur
Trou 5 (bas) -----> Main droite - Annulaire
```

### Doigtés principaux (0=fermé, 1=ouvert)

| Note | MIDI | Doigté  | Octave | Airflow % |
|------|------|---------|--------|-----------|
| D5   | 74   | 000000  | 1      | 0-50      |
| E5   | 76   | 000001  | 1      | 0-50      |
| F#5  | 78   | 000011  | 1      | 0-55      |
| G5   | 79   | 000111  | 1      | 0-60      |
| A5   | 81   | 001111  | 1      | 0-60      |
| B5   | 83   | 011111  | 1      | 0-65      |
| C#6  | 85   | 111111  | 1      | 0-70      |
| D6   | 86   | 000000  | 2      | 30-80     |
| E6   | 88   | 000001  | 2      | 30-80     |
| F#6  | 90   | 000011  | 2      | 35-85     |
| G6   | 91   | 000111  | 2      | 35-85     |
| A6   | 93   | 001111  | 2      | 40-90     |
| B6   | 95   | 011111  | 2      | 40-90     |
| C#7  | 97   | 111111  | 2      | 45-95     |
| D7   | 98   | 000000  | 3      | 50-100    |

### Personnalisation des doigtés

Pour modifier les doigtés, éditer `settings_irish_flute.h` :

```cpp
const NoteDefinition NOTES[NUMBER_NOTES] = {
  // MIDI  Doigtés        Min%  Max%
  {  74,  {0,0,0,0,0,0},  0,   50  },  // Modifier ici
  // ...
};
```

### Ajustement des servos doigts

Modifier les angles et directions dans `FINGERS[]` :

```cpp
const FingerConfig FINGERS[NUMBER_SERVOS_FINGER] = {
  // PCA  Fermé  Sens
  {  0,   90,   -1  },  // Ajuster angle fermé
  {  1,   95,    1  },  // Ajuster sens rotation
  // ...
};
```

**Paramètres à ajuster:**
- `closedAngle` : Angle quand le trou est fermé (0-180°)
- `direction` :
  - `1` = rotation horaire pour ouvrir
  - `-1` = rotation anti-horaire pour ouvrir
- `ANGLE_OPEN` : Degrés d'ouverture (défaut: 30°)

---

## Créer une configuration pour un nouvel instrument

### Template de base

1. Copier `settings_irish_flute.h` vers `settings_mon_instrument.h`
2. Modifier les constantes:

```cpp
// Adapter nombre de trous
#define NUMBER_SERVOS_FINGER 6    // Votre nombre

// Adapter nombre de notes
#define NUMBER_NOTES 15           // Votre nombre

// Modifier les doigtés
const FingerConfig FINGERS[NUMBER_SERVOS_FINGER] = {
  // Vos servos...
};

// Modifier la table des notes
const NoteDefinition NOTES[NUMBER_NOTES] = {
  // Vos notes...
};
```

3. Inclure dans `Servo_flute_v3.ino`:
```cpp
#include "settings_mon_instrument.h"
```

---

## Exemples d'autres instruments possibles

### Tin Whistle (6 trous, comme Irish flute)
- Même configuration que `settings_irish_flute.h`
- Ajuster uniquement les pourcentages d'airflow selon la résistance

### Low Whistle (6 trous, tonalité D grave)
- Même doigtés que Irish flute
- Notes MIDI une octave plus bas (62-86)
- Airflow réduit (diviser les % par 1.5)

### Flûte traversière (clés simplifiées)
- Plus de servos (10-15)
- Doigtés système Boehm
- Airflow très fin (20-60° au lieu de 60-100°)

---

## Vérification et Debug

Avec `DEBUG = 1`, vérifier au démarrage:

```
========================================
   SERVO FLUTE V3 - INITIALISATION
========================================
...
Configuration:
  - Notes jouables: 15 (MIDI 74 - 98)
  - Servos doigts: 6
  - Délai servos→solénoïde: 105 ms
  - Taille queue: 16 événements
```

Pendant le jeu:
```
DEBUG: FingerController - Note MIDI: 74
DEBUG: AirflowController - Note MIDI: 74 | Vel: 80 | Range: 0%-50% (60°-75°) | Angle: 71
```

---

## Questions fréquentes

**Q: Puis-je mixer les deux configurations?**
R: Non, un seul fichier settings à la fois. Mais vous pouvez créer des notes hybrides.

**Q: Comment ajouter une note intermédiaire?**
R: Augmenter `NUMBER_NOTES` et ajouter l'entrée dans `NOTES[]`.

**Q: Les pourcentages ne donnent pas assez d'air?**
R: Modifier `SERVO_AIRFLOW_MIN` et `SERVO_AIRFLOW_MAX` dans settings.

**Q: Un servo tourne dans le mauvais sens?**
R: Inverser `direction` de 1 à -1 (ou vice-versa) dans `FINGERS[]`.

**Q: L'angle fermé n'est pas assez fermé?**
R: Ajuster `closedAngle` dans `FINGERS[]` (augmenter ou diminuer selon `direction`).

---

## Support

Pour toute question sur l'adaptation à un nouvel instrument, consulter:
- `CONFIGURATION_GUIDE.md` - Guide détaillé des paramètres
- `TIMING_ANTICIPATION.md` - Explication du timing
- `SOLENOID_PWM.md` - Configuration solénoïde
