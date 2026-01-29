# Calibration Tool - Servo Flute V3

Outil de calibration pour la Servo Flute, permettant de calibrer les servos doigts et les plages airflow de manière guidée et progressive.

## 📋 Vue d'Ensemble

Cet outil vous guide à travers le processus de calibration complet :
1. **Calibration servos doigts** : Angle fermé + sens de rotation pour chaque doigt
2. **Calibration airflow** : Plage min/max pour chaque note jouable
3. **Génération code** : Code C++ formaté prêt à copier dans `settings.h`

## 🔧 Matériel Requis

- Arduino Leonardo/Micro (USB MIDI natif)
- PCA9685 PWM Driver (I2C)
- Servos SG90 montés sur l'instrument
- Solénoïde connecté (pin 13)
- Alimentation air fonctionnelle
- Câble USB pour connexion PC

## 📦 Installation

### 1. Bibliothèques Arduino

Installer les bibliothèques suivantes via le Library Manager :
- **Adafruit PWM Servo Driver Library**

### 2. Téléversement

1. Ouvrir `Calibration_Tool.ino` dans l'IDE Arduino
2. Sélectionner votre carte (Arduino Leonardo/Micro)
3. Sélectionner le port COM approprié
4. Téléverser le sketch

### 3. Lancement

1. Ouvrir le Serial Monitor (115200 baud)
2. Suivre les instructions à l'écran

## 🎯 Utilisation

### Menu Principal

```
========================================
       MENU PRINCIPAL
========================================
1. Calibrer servos doigts (FINGERS)
2. Calibrer plages airflow (NOTES)
3. Afficher configuration actuelle
4. Générer settings.h final
========================================
```

### Phase 1 : Calibration Servos Doigts

**Objectif** : Trouver l'angle fermé et le sens de rotation pour chaque doigt.

**Processus (pour chaque doigt) :**

1. **Étape 1/3 - Angle fermé**
   - Ajuster l'angle avec `+` / `-` (±1°)
   - Ajuster rapide avec `>` / `<` (±5°)
   - Tester la position avec `t`
   - Sauvegarder avec `s`
   - **But** : Trou complètement bouché, sans forcer le servo

2. **Étape 2/3 - Sens de rotation**
   - Le système teste les deux sens automatiquement
   - Valider le sens qui **ouvre** le trou (ne le ferme pas plus)
   - Répondre `o` (oui) ou `n` (non)

3. **Étape 3/3 - Vérification**
   - Affichage du résumé
   - Test optionnel (oscillation ouvert/fermé)
   - Confirmation finale

**Commandes principales :**
```
+         : Augmenter angle (+1°)
-         : Diminuer angle (-1°)
> ou ]    : Augmenter rapide (+5°)
< ou [    : Diminuer rapide (-5°)
t         : Tester position actuelle
s         : Sauvegarder et continuer
```

### Phase 2 : Calibration Plages Airflow

**Pré-requis** : Les servos doigts doivent être calibrés.

**Objectif** : Trouver le % minimum et maximum d'airflow pour chaque note.

**Processus (pour chaque note) :**

1. **Préparation**
   - Les doigtés sont appliqués automatiquement
   - Le solénoïde s'ouvre
   - Le servo airflow est prêt

2. **Étape 1/2 - airflowMinPercent**
   - Trouver le % **minimum** pour que la note sonne
   - Ajuster avec `+` / `-` / `>` / `<`
   - Tester avec `t` (joue la note 2 secondes)
   - Sauvegarder avec `s`
   - **But** : Note stable et juste au minimum d'air

3. **Étape 2/2 - airflowMaxPercent**
   - Trouver le % **maximum** avant sur-soufflage
   - Même commandes que l'étape 1
   - **But** : Note stable sans monter d'octave ni siffler

4. **Vérification**
   - Affichage du résumé
   - Confirmation finale

**Conseils pratiques :**
- Testez fréquemment avec `t`
- Cherchez le seuil exact (note qui commence à sonner/siffler)
- Notez que le système saute automatiquement à 50% pour l'étape 2 (gain de temps)

### Phase 3 : Génération du Code

Une fois toutes les calibrations terminées :

1. Menu principal → Option 4
2. Le code C++ formaté s'affiche dans le Serial Monitor
3. Copier le code généré
4. Coller dans `Servo_flute_v3/settings.h` aux sections appropriées

**Format du code généré :**

```cpp
/*******************************************************************************
------------------   CONFIGURATION SERVOS DOIGTS       ----------------------
******************************************************************************/
const FingerConfig FINGERS[NUMBER_SERVOS_FINGER] = {
  // PCA  Fermé  Sens
  {  0,   92,   -1  },  // Trou 1 (haut)
  {  1,   95,    1  },  // Trou 2
  ...
};

/*******************************************************************************
-----------------   CONFIGURATION DES NOTES JOUABLES   ----------------------
******************************************************************************/
const NoteDefinition NOTES[NUMBER_NOTES] = {
  // MIDI  Doigtés (6 trous)  Min%  Max%
  {  82,  {0,1,1,1,1,1},  7,   60  },  // A#5 (La#5)
  {  83,  {1,1,1,1,1,1},  0,   48  },  // B5  (Si5)
  ...
};
```

## 📝 Configuration Template

Le fichier `settings_template.h` contient les valeurs par défaut :

### Modifiable (avant calibration)
- `NUMBER_SERVOS_FINGER` : Nombre de doigts (défaut: 6)
- `NUMBER_NOTES` : Nombre de notes (défaut: 14)
- Canaux PCA des servos (`FINGERS_TEMPLATE[].pcaChannel`)
- Doigtés théoriques (`NOTES_TEMPLATE[].fingerPattern`)
- Notes MIDI (`NOTES_TEMPLATE[].midiNote`)

### Calibré (automatiquement)
- Angles fermés (`closedAngle`)
- Sens de rotation (`direction`)
- Plages airflow (`airflowMinPercent`, `airflowMaxPercent`)

## 🔍 Dépannage

### Le servo ne bouge pas
- Vérifier les connexions I2C (SDA, SCL)
- Vérifier l'alimentation PCA9685
- Vérifier le canal PCA dans `settings_template.h`

### Le solénoïde ne s'ouvre pas
- Vérifier la connexion pin 13
- Vérifier l'alimentation du solénoïde
- Vérifier `SOLENOID_ACTIVE_HIGH` dans `settings_template.h`

### La note ne sonne pas
- Vérifier l'alimentation air
- Vérifier que le solénoïde est ouvert
- Augmenter le % airflow progressivement
- Vérifier les doigtés (trous bien fermés/ouverts)

### Serial Monitor ne répond pas
- Vérifier le baud rate (doit être 115200)
- Appuyer sur le bouton Reset de l'Arduino
- Vérifier que le port COM est correct

## 📂 Structure du Projet

```
Calibration_Tool/
├── Calibration_Tool.ino          # Sketch principal
├── settings_template.h            # Template de configuration
├── CalibrationManager.h/cpp       # Chef d'orchestre
├── FingerCalibrator.h/cpp         # Calibration servos doigts
├── AirflowCalibrator.h/cpp        # Calibration airflow
├── OutputGenerator.h/cpp          # Génération code C++
└── README.md                      # Ce fichier
```

## 🎓 Conseils d'Utilisation

### Première Calibration
1. Commencez par un seul doigt pour vous familiariser
2. Notez les valeurs sur papier au cas où
3. Prenez votre temps, la précision est importante
4. Testez chaque configuration avant de valider

### Recalibration Partielle
Si vous voulez recalibrer seulement quelques éléments :
1. Option 1 : Recalibrer tout et ignorer les valeurs non modifiées
2. Option 2 : Copier manuellement les nouvelles valeurs dans `settings.h`

### Optimisation
- Calibrez dans un environnement calme (pour entendre les notes)
- Assurez-vous que l'alimentation air est stable
- Calibrez par ordre croissant (notes graves → aiguës)
- Notez les valeurs si vous testez plusieurs configurations

## 🔄 Workflow Complet

```
1. Installation
   ↓
2. Lancement → Menu principal
   ↓
3. Option 1: Calibrer servos doigts
   ├─ Doigt 1: Angle fermé + Sens
   ├─ Doigt 2: Angle fermé + Sens
   ├─ ...
   └─ Doigt 6: Angle fermé + Sens
   ↓
4. Option 2: Calibrer notes
   ├─ Note A#5: Min% + Max%
   ├─ Note B5:  Min% + Max%
   ├─ ...
   └─ Note G7:  Min% + Max%
   ↓
5. Option 4: Générer code
   ↓
6. Copier dans settings.h
   ↓
7. Téléverser Servo_flute_v3
   ↓
8. ✓ Prêt à jouer!
```

## 📄 Licence

Ce projet fait partie du système Servo Flute V3.

## 👤 Auteur

Servo-Flute Project - 2026

## 🆘 Support

Pour toute question ou problème, référez-vous à la documentation principale du projet Servo Flute V3.
