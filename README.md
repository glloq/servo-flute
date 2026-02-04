# Servo Flute V3 🎵

**Une flûte robotique contrôlée par MIDI avec support breath controller**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Arduino](https://img.shields.io/badge/Arduino-Leonardo%2FMicro-00979D.svg)](https://www.arduino.cc/)
[![Version](https://img.shields.io/badge/Version-3.0-blue.svg)](https://github.com/glloq/servo-flute)

---

## 📋 Vue d'ensemble

La **Servo Flute V3** est un instrument robotique qui transforme des messages MIDI en sons de flûte acoustique. Utilisant des servomoteurs pour actionner les doigts et contrôler le débit d'air, elle offre un contrôle expressif comparable à une flûte jouée par un humain.

### Caractéristiques principales

- ✅ **Contrôle MIDI complet** - 8 Control Changes implémentés
- ✅ **Breath Controller (CC2)** - Contrôle dynamique du souffle en temps réel
- ✅ **Irish Flute 6 trous** - 14 notes jouables (A#5 - G7)
- ✅ **Vibrato optimisé** - sin() LUT pour CPU efficace
- ✅ **Watchdog timer** - Auto-restart en cas de blocage
- ✅ **Outil de calibration** - Interface Serial Monitor intuitive
- ✅ **Documentation complète** - Architecture, MIDI, Configuration

---

## 🎵 Démonstration

> [!NOTE]
> Ajoutez ici une vidéo démo ou un lien vers des enregistrements audio

---

## 🚀 Démarrage Rapide

### Matériel Requis

#### Électronique
- **Arduino Leonardo ou Micro** (ATmega32u4 avec USB-MIDI natif)
- **PCA9685** - Module PWM 16 canaux
- **7 servomoteurs 9g** (6 doigts + 1 airflow)
- **Solénoïde 12V** - Valve pneumatique
- **Alimentation 5V** - 10A minimum (servos)
- **Alimentation 12V** - 2A (solénoïde)

#### Mécanique
- **Irish flute** ou **flûte à bec**
- **Fil de fer 1mm** - Connexion servos → doigts
- **Mousse** - Bande 5mm largeur, 3mm épaisseur (doigts)
- **Supports imprimés 3D** - Fichiers dans `/stl/`
- **Planche bois** - 40x25cm support général
- **Vis bois** - 3x35mm (2 par servo)

### Logiciel

```bash
# 1. Cloner le repo
git clone https://github.com/glloq/servo-flute.git
cd servo-flute

# 2. Ouvrir avec Arduino IDE
# Ouvrir Servo_flute_v3/Servo_flute_v3.ino

# 3. Installer bibliothèques
# Arduino IDE → Sketch → Include Library → Manage Libraries
# Installer : Adafruit PWM Servo Driver Library, MIDIUSB
```

### Calibration

```bash
# 1. Ouvrir outil calibration
# Ouvrir Calibration_Tool/Calibration_Tool.ino

# 2. Upload sur Arduino

# 3. Serial Monitor (115200 baud)
# Suivre instructions pour calibrer servos + notes

# 4. Copier code généré dans Servo_flute_v3/settings.h
```

**Documentation complète :** [Calibration_Tool/README.md](Calibration_Tool/README.md)

---

## 🎹 Contrôles MIDI

### Control Changes Implémentés

| CC | Nom | Fonction |
|----|-----|----------|
| **CC1** | Modulation | Vibrato (±8°, 6Hz) |
| **CC2** | Breath Controller | Contrôle dynamique souffle (remplace velocity) |
| **CC7** | Volume | Réduit limite haute de la plage |
| **CC11** | Expression | Nuances dynamiques (crescendo/diminuendo) |
| **CC74** | Brightness | Stocké (usage futur) |
| **CC120** | All Sound Off | Arrêt d'urgence |
| **CC121** | Reset All Controllers | Réinitialise tous les CC |
| **CC123** | All Notes Off | Identique à CC 120 |

### Ordre d'Application

```
CC7 (Volume) → CC2 (Breath) OU Velocity → CC11 (Expression) → CC1 (Vibrato)
      ↓                 ↓                        ↓                   ↓
 Réduit plage      Source airflow           Nuances dans      Oscillation
 haute globale     (prioritaire)            plage réduite     autour angle
```

**Documentation détaillée :** [docs/MIDI_CC_IMPLEMENTATION.md](docs/MIDI_CC_IMPLEMENTATION.md)

---

## 🎛️ CC2 Breath Controller

### Qu'est-ce que c'est ?

Le **Breath Controller** (CC2) permet de contrôler le souffle en temps réel via :
- **Breath controller physique** (Yamaha BC3, TEControl BBC2)
- **Automation DAW** (courbes de souffle dessinées)

### Fonctionnalités

- ✅ **Lissage** - Moyenne glissante (5 valeurs) contre jitter
- ✅ **Courbe exponentielle** - CC2^1.4 pour réponse naturelle
- ✅ **Seuil silence** - CC2 < 10 → valve fermée
- ✅ **Fallback velocity** - Utilise velocity si CC2 absent > 1s
- ✅ **Rate limiting** - 50 CC2/sec (haute fréquence)

### Cas d'usage

**Musicien avec breath controller :**
```
Yamaha BC3 → USB-MIDI → Servo-flute
→ Contrôle souffle direct en temps réel
```

**Automation DAW :**
```
Ableton/Logic → Automation CC2 → Servo-flute
→ Courbes de souffle pré-enregistrées
```

**Documentation complète :** [docs/CC2_BREATH_CONTROLLER.md](docs/CC2_BREATH_CONTROLLER.md)

---

## 📚 Documentation

### Index Complet

**[📖 Documentation complète dans /docs/](docs/README.md)**

#### 🏁 Démarrage
- [README_V3.md](docs/README_V3.md) - Vue d'ensemble V3
- [CONFIGURATION_GUIDE.md](docs/CONFIGURATION_GUIDE.md) - Configuration complète
- [Calibration_Tool/README.md](Calibration_Tool/README.md) - Guide calibration

#### 🏗️ Architecture
- [ARCHITECTURE.md](docs/ARCHITECTURE.md) - Architecture globale
  - Structure classes
  - Flux de données
  - Sécurité et robustesse

#### 🎹 MIDI
- [MIDI_CC_IMPLEMENTATION.md](docs/MIDI_CC_IMPLEMENTATION.md) - Tous les CC
- [CC2_BREATH_CONTROLLER.md](docs/CC2_BREATH_CONTROLLER.md) - CC2 détaillé

#### ⚙️ Optimisations
- [SOLENOID_PWM.md](docs/SOLENOID_PWM.md) - PWM solénoïde
- [VALVE_OPTIMIZATION.md](docs/VALVE_OPTIMIZATION.md) - Optimisation valve
- [TIMING_ANTICIPATION.md](docs/TIMING_ANTICIPATION.md) - Timing et anticipation

#### 🎼 Instruments
- [INSTRUMENTS_GUIDE.md](docs/INSTRUMENTS_GUIDE.md) - Adaptation instruments

---

## 🔧 Configuration Actuelle

### Instrument : Irish Flute 6 Trous

```cpp
// settings.h
#define NUMBER_SERVOS_FINGER 6

const NoteDefinition NOTES[] = {
  {70,  20,  75, 0b111111},  // A#5
  {72,  20,  75, 0b111110},  // C6
  {73,  20,  75, 0b111100},  // C#6
  // ... 14 notes total (A#5 - G7)
};
```

**Caractéristiques :**
- **Tonalité :** C majeur
- **Notes jouables :** 14 (A#5 à G7)
- **Servos doigts :** 6
- **Canal MIDI :** Omni (0) - Écoute tous les canaux

---

## 🏗️ Architecture

### Classes Principales

```
Servo_flute_v3.ino          # Point d'entrée, loop, watchdog
├── MidiHandler             # Réception MIDI USB
├── InstrumentManager       # Orchestration globale
│   ├── EventQueue          # File événements MIDI
│   ├── FingerController    # Contrôle doigts
│   ├── AirflowController   # Contrôle airflow + CC
│   └── NoteSequencer       # Séquençage temporel
└── settings.h              # Configuration centrale
```

### Flux de Données

```
USB MIDI → MidiHandler
             ↓
       InstrumentManager (rate limiting)
             ↓
    ┌────────┴────────┐
    ↓                 ↓
FingerController  AirflowController
    ↓                 ↓
Servos doigts    Servo airflow + Solénoïde
```

**Documentation complète :** [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)

---

## 🔒 Sécurité

### Watchdog Timer

**Protection contre blocage :**
```cpp
wdt_enable(WDTO_4S);  // Watchdog 4 secondes
// loop() doit s'exécuter < 4s sinon auto-restart
```

### État Sûr (initSafeState)

**Appelé AVANT toute initialisation :**
- Solénoïde fermé
- Airflow au repos
- Tous doigts fermés

**Protection :** Crash, reset, power-on

### Rate Limiting

- **CC général :** 10 messages/sec
- **CC2 (Breath) :** 50 messages/sec
- **Exemptions :** CC 120, 121, 123 (urgence)

---

## 📊 Performance

### Latence

| Événement | Latence |
|-----------|---------|
| Note On → Doigts positionnés | < 5ms |
| Note On → Valve ouverte | 30ms (anticipation) |
| CC2 reçu → Airflow ajusté | < 2ms |
| Vibrato update | 1ms (continu) |

### Charge CPU

- **Loop principale :** ~20%
- **CC2 lissage :** ~2%
- **Vibrato sin() LUT :** < 1%
- **Total :** ~25% (marge confortable)

---

## 🛠️ Outils

### Calibration Tool

**Interface Serial Monitor interactive pour calibrer :**
1. Servos doigts (angle fermé/ouvert)
2. Notes (airflowMin%/airflowMax%)
3. Génération code C++ → Copier dans `settings.h`

**Documentation :** [Calibration_Tool/README.md](Calibration_Tool/README.md)

---

## 🤝 Contribution

### Structure du Projet

```
servo-flute/
├── Servo_flute_v3/       # Code principal
├── Calibration_Tool/     # Outil calibration
├── docs/                 # Documentation complète
├── img/                  # Images
├── stl/                  # Fichiers STL 3D
└── README.md             # Ce fichier
```

### Contribuer

1. Fork le projet
2. Créer une branche (`git checkout -b feature/AmazingFeature`)
3. Commit changements (`git commit -m 'Add AmazingFeature'`)
4. Push vers la branche (`git push origin feature/AmazingFeature`)
5. Ouvrir une Pull Request

---

## 📈 Historique

### V3 (2026-01-25 à 2026-02-04)

**Features majeures :**
- ✅ 8 Control Changes MIDI
- ✅ CC2 Breath Controller (contrôle souffle dynamique)
- ✅ Nouvelle logique CC7→CC2→CC11
- ✅ Rate limiting configurable
- ✅ Canal MIDI (omni + spécifique)
- ✅ Vibrato optimisé (sin LUT)
- ✅ Watchdog + état sûr
- ✅ Irish Flute 6 trous
- ✅ Outil calibration

**Commits principaux :**
- `CC2 Breath Controller : Contrôle dynamique souffle` (2026-02-04)
- `Suppression Pitch Bend : Logique incorrecte retirée` (2026-02-04)
- `Nouvelle logique CC7/CC11 : Volume réduit plage avant Expression` (2026-02-04)
- `Améliorations MIDI : Canal, CC étendus, Rate Limiting` (2026-02-04)

---

## 📜 Licence

Ce projet est sous licence **MIT** - Voir le fichier [LICENSE](LICENSE) pour détails.

---

## 🙏 Remerciements

- **Adafruit** - Bibliothèque PCA9685
- **Arduino** - Platform et bibliothèques MIDI
- **Communauté Open Source** - Inspirations et ressources

---

## 📞 Contact & Support

**GitHub Repository :** [https://github.com/glloq/servo-flute](https://github.com/glloq/servo-flute)

**Issues & Questions :** [GitHub Issues](https://github.com/glloq/servo-flute/issues)

**Documentation :** [/docs/README.md](docs/README.md)

---

**Créé par :** [@glloq](https://github.com/glloq)
**Version :** 3.0
**Dernière mise à jour :** 2026-02-04
