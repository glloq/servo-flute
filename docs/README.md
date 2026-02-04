# Documentation Servo Flute V3

Bienvenue dans la documentation complète de la Servo Flute V3 - Une flûte robotique contrôlée par MIDI.

---

## 📚 Index de la Documentation

### 🏁 Démarrage

| Document | Description |
|----------|-------------|
| **[README_V3.md](README_V3.md)** | Vue d'ensemble de la Servo Flute V3 |
| **[CONFIGURATION_GUIDE.md](CONFIGURATION_GUIDE.md)** | Guide de configuration complète |
| **[../Calibration_Tool/README.md](../Calibration_Tool/README.md)** | Guide d'utilisation de l'outil de calibration |

### 🏗️ Architecture et Code

| Document | Description |
|----------|-------------|
| **[ARCHITECTURE.md](ARCHITECTURE.md)** | Architecture globale du système |
| | • Structure des classes |
| | • Flux de données |
| | • Organisation du code |
| | • Sécurité et robustesse |

### 🎹 MIDI et Contrôles

| Document | Description |
|----------|-------------|
| **[MIDI_CC_IMPLEMENTATION.md](MIDI_CC_IMPLEMENTATION.md)** | Tous les Control Changes MIDI |
| | • CC1 (Modulation/Vibrato) |
| | • CC2 (Breath Controller) |
| | • CC7 (Volume) |
| | • CC11 (Expression) |
| | • CC74 (Brightness) |
| | • CC120, CC121, CC123 (Contrôles système) |
| | • Canal MIDI et rate limiting |
| **[CC2_BREATH_CONTROLLER.md](CC2_BREATH_CONTROLLER.md)** | CC2 Breath Controller détaillé |
| | • Fonctionnement technique |
| | • Configuration |
| | • Lissage, courbe exponentielle |
| | • Cas d'usage |
| | • Dépannage |

### ⚙️ Optimisations et Techniques

| Document | Description |
|----------|-------------|
| **[SOLENOID_PWM.md](SOLENOID_PWM.md)** | Contrôle PWM du solénoïde |
| | • PWM activation + maintien |
| | • Réduction consommation |
| | • Durée de vie prolongée |
| **[VALVE_OPTIMIZATION.md](VALVE_OPTIMIZATION.md)** | Optimisation de la valve pneumatique |
| | • Temps de réponse |
| | • Gestion transitions |
| **[TIMING_ANTICIPATION.md](TIMING_ANTICIPATION.md)** | Timing et anticipation mécanique |
| | • Anticipation doigts |
| | • Séquençage événements |
| | • Legato et staccato |

### 🎼 Instruments

| Document | Description |
|----------|-------------|
| **[INSTRUMENTS_GUIDE.md](INSTRUMENTS_GUIDE.md)** | Guide pour différents instruments |
| | • Irish Flute (6 trous) |
| | • Flûte à bec |
| | • Adaptation personnalisée |

---

## 🎯 Parcours Recommandés

### Pour débutant (première utilisation)

1. **[README_V3.md](README_V3.md)** - Comprendre le projet
2. **[CONFIGURATION_GUIDE.md](CONFIGURATION_GUIDE.md)** - Configuration matérielle
3. **[../Calibration_Tool/README.md](../Calibration_Tool/README.md)** - Calibrer l'instrument
4. **[MIDI_CC_IMPLEMENTATION.md](MIDI_CC_IMPLEMENTATION.md)** - Découvrir les contrôles MIDI

### Pour développeur (comprendre le code)

1. **[ARCHITECTURE.md](ARCHITECTURE.md)** - Structure globale
2. **[MIDI_CC_IMPLEMENTATION.md](MIDI_CC_IMPLEMENTATION.md)** - Implémentation MIDI
3. **[CC2_BREATH_CONTROLLER.md](CC2_BREATH_CONTROLLER.md)** - Feature avancée CC2
4. **[TIMING_ANTICIPATION.md](TIMING_ANTICIPATION.md)** - Timing précis

### Pour musicien (utilisation avancée)

1. **[MIDI_CC_IMPLEMENTATION.md](MIDI_CC_IMPLEMENTATION.md)** - Tous les contrôles MIDI
2. **[CC2_BREATH_CONTROLLER.md](CC2_BREATH_CONTROLLER.md)** - Breath controller physique
3. **[INSTRUMENTS_GUIDE.md](INSTRUMENTS_GUIDE.md)** - Adapter à votre instrument

### Pour optimisation et troubleshooting

1. **[SOLENOID_PWM.md](SOLENOID_PWM.md)** - Optimisation solénoïde
2. **[VALVE_OPTIMIZATION.md](VALVE_OPTIMIZATION.md)** - Optimisation valve
3. **[TIMING_ANTICIPATION.md](TIMING_ANTICIPATION.md)** - Réglages timing

---

## 🔧 Configuration Actuelle

**Instrument :** Irish Flute 6 trous
**Tonalité :** C majeur
**Notes jouables :** 14 (A#5 - G7)
**Servos doigts :** 6
**Canal MIDI :** Omni (0) - Écoute tous les canaux

---

## 🎵 Features MIDI Implémentées

### Control Changes (CC)

- ✅ **CC1** - Modulation (Vibrato)
- ✅ **CC2** - Breath Controller (Contrôle dynamique souffle)
- ✅ **CC7** - Volume (Réduction limite haute)
- ✅ **CC11** - Expression (Nuances dynamiques)
- ✅ **CC74** - Brightness (Stocké, usage futur)
- ✅ **CC120** - All Sound Off (Arrêt urgence)
- ✅ **CC121** - Reset All Controllers
- ✅ **CC123** - All Notes Off

### Autres Features MIDI

- ✅ **Canal MIDI** : Omni mode (0) ou spécifique (1-16)
- ✅ **Rate Limiting** : 10 CC/sec général, 50 CC2/sec
- ✅ **Vibrato optimisé** : sin() LUT 256 entrées
- ❌ **Pitch Bend** : Retiré (logique incorrecte)

---

## 📊 Ordre d'Application des Contrôles

```
CC7 (Volume) → CC2 (Breath) OU Velocity → CC11 (Expression) → CC1 (Vibrato)
      ↓                 ↓                        ↓                   ↓
 Réduit plage      Source airflow           Nuances dans      Oscillation
 haute globale     (choix prioritaire)      plage réduite     autour angle
```

**Détails :** Voir [MIDI_CC_IMPLEMENTATION.md](MIDI_CC_IMPLEMENTATION.md)

---

## 🛠️ Outils Disponibles

### Calibration Tool

**Emplacement :** `/Calibration_Tool/`

**Fonction :** Calibration interactive via Serial Monitor

**Workflow :**
1. Calibrer servos doigts (angle + direction)
2. Calibrer notes (airflowMin% + airflowMax%)
3. Générer code C++ formaté
4. Copier-coller dans `settings.h`

**Documentation :** [../Calibration_Tool/README.md](../Calibration_Tool/README.md)

---

## 🔒 Sécurité

### Watchdog Timer
- **Timeout :** 4 secondes
- **Fonction :** Auto-restart si code bloqué
- **Implémentation :** `wdt_enable(WDTO_4S)` + `wdt_reset()` dans loop

### État Sûr (initSafeState)
- **Appelé :** Au démarrage AVANT toute initialisation
- **Actions :** Solénoïde fermé, airflow repos, doigts fermés
- **Protection :** Crash, reset, power-on

### Rate Limiting
- **CC général :** 10 messages/sec
- **CC2 (Breath) :** 50 messages/sec
- **Exemptions :** CC 120, 121, 123 (urgence)

---

## 📈 Historique des Versions

### V3 (Actuelle)

**Date :** 2026-01-25 à 2026-02-04

**Features majeures :**
- ✅ Control Changes MIDI complets (8 CC)
- ✅ CC2 Breath Controller (contrôle souffle dynamique)
- ✅ Nouvelle logique CC7→CC2→CC11
- ✅ Rate limiting configurableble
- ✅ Canal MIDI (omni + spécifique)
- ✅ Vibrato optimisé (sin LUT)
- ✅ Watchdog timer + état sûr
- ✅ Configuration Irish Flute 6 trous
- ✅ Outil calibration standalone

**Commits principaux :**
- `CC2 Breath Controller : Contrôle dynamique souffle` (2026-02-04)
- `Suppression Pitch Bend : Logique incorrecte retirée` (2026-02-04)
- `Nouvelle logique CC7/CC11 : Volume réduit plage avant Expression` (2026-02-04)
- `Améliorations MIDI : Canal, CC étendus, Rate Limiting` (2026-02-04)

---

## 🤝 Contribution

### Structure Documentation

**Principe :** Un document = Un sujet précis

**Organisation :**
```
docs/
├── README.md                      # Index (ce fichier)
├── ARCHITECTURE.md                # Architecture code
├── MIDI_CC_IMPLEMENTATION.md      # Control Changes généraux
├── CC2_BREATH_CONTROLLER.md       # CC2 spécifique (détaillé)
├── CONFIGURATION_GUIDE.md         # Configuration matérielle
├── INSTRUMENTS_GUIDE.md           # Adaptation instruments
├── SOLENOID_PWM.md                # Technique solénoïde
├── TIMING_ANTICIPATION.md         # Technique timing
├── VALVE_OPTIMIZATION.md          # Optimisation valve
└── README_V3.md                   # Vue d'ensemble V3
```

**Ajout de documentation :**
1. Créer document dans `/docs/`
2. Format Markdown
3. Mettre à jour cet index (README.md)
4. Commit avec message descriptif

---

## 📞 Support

**GitHub Repository :** [https://github.com/glloq/servo-flute](https://github.com/glloq/servo-flute)

**Issues :** Utiliser GitHub Issues pour :
- Bugs
- Questions
- Suggestions d'amélioration
- Demandes de documentation

---

**Créé le :** 2026-02-04
**Version Servo Flute :** V3
**Dernière mise à jour :** 2026-02-04
