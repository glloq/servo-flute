# CC2 Breath Controller - Contrôle Dynamique du Souffle

## 📋 Vue d'ensemble

Le **CC2 (Breath Controller)** permet un contrôle dynamique et expressif du souffle en temps réel sur la servo-flute. Il remplace la velocity MIDI pour offrir un contrôle continu de l'airflow, transformant la servo-flute en véritable instrument à vent MIDI jouable avec un breath controller physique ou via automation DAW.

**Implémentation :** Option 1 - Remplacement Velocity
**Date d'implémentation :** 2026-02-04
**Statut :** ✅ Complet et fonctionnel

---

## 🎯 Qu'est-ce qu'un Breath Controller ?

### Définition

Un breath controller est un capteur de pression buccale qui convertit l'intensité du souffle d'un musicien en messages MIDI CC2 (valeurs 0-127). Il permet de contrôler des instruments MIDI comme on soufflerait dans un instrument à vent acoustique.

### Contrôleurs physiques compatibles

- **Yamaha BC3A / BC3** : Capteur de pression avec tube
- **TEControl BBC2** : Breath controller USB moderne
- **AKAI EWI / Yamaha WX5** : Wind controllers avec capteur intégré
- **Automation DAW** : Courbes de souffle dessinées (Ableton, Logic, etc.)

---

## 🔧 Fonctionnement Technique

### Principe de base

```
Musicien souffle → Capteur pression → CC2 MIDI (0-127) → Servo-flute → Airflow dynamique
```

**Différence avec Velocity :**
- **Velocity** : Fixe, définie au Note On (une seule valeur par note)
- **CC2** : Continu, envoyé en temps réel (20-100 msg/sec pendant la note)

### Ordre d'application

```
CC7 (Volume) → CC2 (Breath) OU Velocity → CC11 (Expression) → CC1 (Vibrato)
         ↓              ↓                        ↓                    ↓
   Réduit plage    Source airflow         Nuances dans      Oscillation
   haute globale   principale (choix)     plage réduite     autour angle
```

---

## ⚙️ Configuration (settings.h)

### Constantes disponibles

```cpp
/*******************************************************************************
--------------------  BREATH CONTROLLER (CC2) SETTINGS  ----------------------
******************************************************************************/

#define CC2_ENABLED true                  // Activer/désactiver CC2
#define CC2_RATE_LIMIT_PER_SECOND 50      // Max 50 CC2/sec (haute fréquence)
#define CC2_SILENCE_THRESHOLD 10          // CC2 < 10 → valve fermée
#define CC2_SMOOTHING_BUFFER_SIZE 5       // Buffer lissage (moyenne glissante)
#define CC2_RESPONSE_CURVE 1.4            // Courbe exponentielle (1.0-2.0)
#define CC2_TIMEOUT_MS 1000               // Timeout fallback velocity (ms)
```

### Paramètres expliqués

#### CC2_ENABLED
- **Type :** `true` / `false`
- **Défaut :** `true`
- **Effet :** Active ou désactive complètement le breath controller
- **Usage :** Mettre à `false` pour revenir au comportement velocity classique

#### CC2_RATE_LIMIT_PER_SECOND
- **Type :** Entier (1-100)
- **Défaut :** `50`
- **Effet :** Nombre maximum de messages CC2 traités par seconde
- **Recommandations :**
  - **30** : Automation DAW légère
  - **50** : Breath controller physique standard (défaut)
  - **100** : Breath controller haute performance (peut créer jitter)

#### CC2_SILENCE_THRESHOLD
- **Type :** Entier (0-127)
- **Défaut :** `10`
- **Effet :** Valeurs CC2 < seuil → considérées comme silence (valve fermée)
- **Recommandations :**
  - **5** : Seuil très bas (capte souffle très faible)
  - **10** : Seuil standard (défaut, évite bruits parasites)
  - **20** : Seuil élevé (nécessite souffle plus fort pour déclencher)

#### CC2_SMOOTHING_BUFFER_SIZE
- **Type :** Entier (1-10)
- **Défaut :** `5`
- **Effet :** Taille du buffer de lissage (moyenne glissante)
- **Recommandations :**
  - **3** : Lissage léger, réponse très rapide (peut avoir jitter)
  - **5** : Lissage standard (défaut, bon compromis)
  - **7-10** : Lissage fort, réponse plus lente mais très stable

#### CC2_RESPONSE_CURVE
- **Type :** Float (1.0-2.0)
- **Défaut :** `1.4`
- **Effet :** Courbe exponentielle appliquée : `CC2^valeur`
- **Comportement :**
  - **1.0** : Linéaire (pas de courbe)
  - **1.2-1.4** : Légèrement exponentiel (souffle faible = contrôle fin)
  - **1.5-1.8** : Très exponentiel (souffle faible = beaucoup de contrôle)
  - **2.0** : Quadratique (souffle faible = maximum de contrôle)

**Exemple courbe 1.4 :**
```
CC2 raw → Après courbe
10 → 6     (souffle faible = contrôle très fin)
50 → 35    (souffle moyen = contrôle progressif)
100 → 83   (souffle fort = puissance)
127 → 127  (souffle max = max)
```

#### CC2_TIMEOUT_MS
- **Type :** Entier (0-5000 ms)
- **Défaut :** `1000` (1 seconde)
- **Effet :** Temps sans CC2 avant fallback sur velocity
- **Recommandations :**
  - **0** : Pas de timeout (toujours CC2 si reçu, sinon velocity)
  - **500** : Timeout court (fallback rapide)
  - **1000** : Timeout standard (défaut)
  - **2000+** : Timeout long (breath controller temporairement inactif OK)

---

## 🎵 Fonctionnalités

### 1. Lissage (Smoothing)

**Problème :** Les breath controllers envoient des données très rapidement (50-100/sec) avec micro-variations → Jitter du servo

**Solution :** Buffer circulaire avec moyenne glissante

```cpp
// Buffer circulaire de 5 valeurs (défaut)
byte _cc2SmoothingBuffer[5] = {120, 122, 121, 123, 122};

// Calcul moyenne
smoothedCC2 = (120 + 122 + 121 + 123 + 122) / 5 = 121.6 ≈ 122

// Nouvelle valeur reçue (124) → Remplace la plus ancienne
_cc2SmoothingBuffer = {122, 121, 123, 122, 124};
smoothedCC2 = 122.4 ≈ 122
```

**Avantages :**
- Élimine les micro-variations
- Servo suit doucement les changements
- Pas de saccades mécaniques

### 2. Courbe Exponentielle

**Problème :** Relation linéaire CC2 → airflow peu naturelle

**Solution :** Courbe exponentielle `CC2^1.4`

```cpp
float normalizedCC2 = smoothedCC2 / 127.0;     // 0.0 - 1.0
float curvedCC2 = pow(normalizedCC2, 1.4);     // Courbe exponentielle
byte finalCC2 = curvedCC2 * 127;               // Retour 0-127
```

**Effet musical :**

| CC2 raw | Après courbe | Commentaire |
|---------|-------------|-------------|
| 20 | 11 | Souffle faible = GRAND contrôle fin |
| 40 | 26 | Contrôle progressif |
| 60 | 43 | Zone intermédiaire |
| 80 | 62 | Souffle fort commence |
| 100 | 83 | Puissance |
| 127 | 127 | Maximum inchangé |

**Avantages :**
- Souffle faible → Contrôle très précis (nuances pianissimo)
- Souffle fort → Puissance directe (fortissimo)
- Comportement naturel comme une vraie flûte

### 3. Seuil de Silence

**Problème :** CC2 = 0-9 → Bruits parasites, souffle trop faible

**Solution :** Seuil à 10 (configurable)

```cpp
if (smoothedCC2 < CC2_SILENCE_THRESHOLD) {
  airflowSource = 0;                    // Silence
  setAirflowServoAngle(SERVO_AIRFLOW_OFF);
  closeSolenoid();                      // Fermer valve
  return;
}
```

**Avantages :**
- Pas de bruits parasites avec souffle très faible
- Silence franc (valve fermée)
- Comportement naturel : pas de souffle = pas de son

### 4. Fallback Velocity

**Problème :** Breath controller déconnecté/absent → Aucun son

**Solution :** Timeout automatique avec fallback sur velocity

```cpp
unsigned long timeSinceCC2 = millis() - _lastCC2Time;

if (CC2_TIMEOUT_MS > 0 && timeSinceCC2 > CC2_TIMEOUT_MS) {
  // Timeout : fallback sur velocity
  airflowSource = velocity;

  if (DEBUG) {
    Serial.print("DEBUG: CC2 timeout (");
    Serial.print(timeSinceCC2);
    Serial.print("ms) - Fallback velocity");
  }
}
```

**Avantages :**
- Fonctionne sans breath controller (utilise velocity classique)
- Transition automatique transparente
- Utile si breath controller temporairement inactif

### 5. Rate Limiting Séparé

**Problème :** CC2 haute fréquence (50/sec) vs autres CC (10/sec)

**Solution :** Rate limit séparé pour CC2

```cpp
// InstrumentManager.cpp
if (ccNumber == 2) {
  // Rate limiting CC2 : 50 msg/sec
  if (_cc2Count > CC2_RATE_LIMIT_PER_SECOND) {
    return;  // Ignorer
  }
} else {
  // Rate limiting autres CC : 10 msg/sec
  if (_ccCount > CC_RATE_LIMIT_PER_SECOND) {
    return;  // Ignorer
  }
}
```

**Avantages :**
- CC2 peut envoyer 50/sec (haute fréquence nécessaire)
- Autres CC limités à 10/sec (suffisant)
- Protection CPU optimale

---

## 🎹 Cas d'usage

### Cas 1 : Musicien avec breath controller physique

**Setup :**
```
Yamaha BC3 → USB-MIDI → DAW → Servo-flute
```

**Comportement :**
1. Musicien joue note C6 (Note On, velocity 100)
2. Velocity donne attaque initiale → Angle de base
3. Musicien commence à souffler → CC2 reçu (20/sec)
4. CC2 remplace velocity immédiatement
5. Musicien module souffle en temps réel :
   - Souffle faible (CC2 = 30) → Son doux, contrôle fin
   - Souffle moyen (CC2 = 70) → Son normal
   - Souffle fort (CC2 = 120) → Son puissant
6. Crescendo/diminuendo naturels par pression buccale

**Résultat :** La servo-flute répond exactement comme si le musicien soufflait dedans directement !

### Cas 2 : Automation DAW sans breath controller

**Setup :**
```
Ableton Live → Automation CC2 dessinée → Servo-flute
```

**Comportement :**
1. DAW envoie Note On + CC2 automatisé (courbe dessinée)
2. CC2 simule courbe de souffle réaliste :
   - Attaque : CC2 monte de 0 → 100 (0.1s)
   - Sustain : CC2 varie 90-110 (vibrations naturelles)
   - Release : CC2 descend de 100 → 0 (0.2s)
3. Servo suit précisément la courbe automatisée

**Résultat :** Performance enregistrée avec dynamique de souffle naturelle, impossible avec velocity seule.

### Cas 3 : Hybride (automation + live breath)

**Setup :**
```
DAW (CC7 + CC11 automation) + Musicien (CC2 breath live) → Servo-flute
```

**Comportement :**
1. DAW automatise :
   - CC7 : Volume global du morceau (fade in/out, variations section)
   - CC11 : Phrasé musical pré-enregistré (crescendos écrits)
2. Musicien contrôle en direct :
   - CC2 : Nuances en temps réel par souffle
3. Trois niveaux simultanés :
   - **CC7** = Structure globale (automation master)
   - **CC11** = Phrasé écrit (automation track)
   - **CC2** = Interprétation live (breath direct)

**Résultat :** Maximum d'expressivité - automation compositionnelle + interprétation performative !

---

## 🔬 Logique d'implémentation

### Flux de données

```
1. CC2 MIDI reçu (MidiHandler)
         ↓
2. InstrumentManager.handleControlChange(2, ccValue)
         ↓
3. Rate limiting vérifié (50/sec max)
         ↓
4. AirflowController.updateCC2Breath(ccValue)
         ↓
5. Ajout au buffer circulaire
         ↓
6. Timestamp mis à jour (_lastCC2Time)
         ↓
7. Note jouée → setAirflowForNote()
         ↓
8. Vérification timeout CC2
         ↓
9a. CC2 actif : Calcul moyenne lissée
         ↓
10a. Application courbe exponentielle
         ↓
11a. Vérification seuil silence
         ↓
12a. CC2 utilisé comme airflowSource

    OU

9b. CC2 timeout : Fallback velocity
         ↓
12b. Velocity utilisée comme airflowSource
         ↓
13. CC7 → airflowSource → CC11 → Vibrato
```

### Code simplifié

```cpp
// AirflowController.cpp - setAirflowForNote()

// 1. Calculer moyenne lissée CC2
uint16_t sum = 0;
for (uint8_t i = 0; i < _cc2BufferCount; i++) {
  sum += _cc2SmoothingBuffer[i];
}
byte smoothedCC2 = sum / _cc2BufferCount;

// 2. Seuil silence
if (smoothedCC2 < CC2_SILENCE_THRESHOLD) {
  airflowSource = 0;  // Silence
  closeSolenoid();
  return;
}

// 3. Courbe exponentielle
float normalizedCC2 = smoothedCC2 / 127.0;
float curvedCC2 = pow(normalizedCC2, CC2_RESPONSE_CURVE);
airflowSource = (byte)(curvedCC2 * 127);

// 4. Utiliser comme source airflow
baseAngle = map(airflowSource, 1, 127, minAngle, effectiveMaxAngle);
```

---

## 🎛️ Réglages Recommandés

### Pour breath controller physique (Yamaha BC3, TEControl)

```cpp
#define CC2_ENABLED true
#define CC2_RATE_LIMIT_PER_SECOND 50      // Haute fréquence
#define CC2_SILENCE_THRESHOLD 10          // Standard
#define CC2_SMOOTHING_BUFFER_SIZE 5       // Bon équilibre
#define CC2_RESPONSE_CURVE 1.4            // Naturel
#define CC2_TIMEOUT_MS 1000               // 1s avant fallback
```

### Pour automation DAW uniquement

```cpp
#define CC2_ENABLED true
#define CC2_RATE_LIMIT_PER_SECOND 30      // Fréquence modérée OK
#define CC2_SILENCE_THRESHOLD 5           // Plus sensible
#define CC2_SMOOTHING_BUFFER_SIZE 3       // Moins de lissage
#define CC2_RESPONSE_CURVE 1.2            // Courbe légère
#define CC2_TIMEOUT_MS 0                  // Pas de timeout
```

### Pour désactiver CC2 (velocity classique)

```cpp
#define CC2_ENABLED false                 // Désactivé
// Les autres paramètres sont ignorés
```

---

## 📊 Comparaison avec Velocity

| Aspect | Velocity | CC2 Breath Controller |
|--------|----------|----------------------|
| **Fréquence** | 1 fois (Note On) | Continue (20-100/sec) |
| **Contrôle** | Statique | Dynamique temps réel |
| **Expressivité** | Fixe par note | Crescendo/diminuendo en cours |
| **Naturel flûte** | Approximation | Simulation exacte |
| **Matériel** | Clavier MIDI standard | Breath controller requis |
| **Automation** | Facile (1 valeur) | Courbes à dessiner |
| **CPU** | Minimal | Modéré (lissage) |

**Conclusion :** CC2 transforme la servo-flute en véritable instrument à vent expressif, au prix d'une complexité accrue.

---

## 🐛 Dépannage

### Problème : Jitter du servo avec CC2

**Symptômes :** Servo saccade, mouvements brusques

**Solutions :**
1. Augmenter `CC2_SMOOTHING_BUFFER_SIZE` à 7-10
2. Réduire `CC2_RATE_LIMIT_PER_SECOND` à 30
3. Vérifier qualité breath controller (capteur bruyant ?)

### Problème : Réponse trop lente

**Symptômes :** Délai entre souffle et son

**Solutions :**
1. Réduire `CC2_SMOOTHING_BUFFER_SIZE` à 3
2. Augmenter `CC2_RATE_LIMIT_PER_SECOND` à 60-80
3. Vérifier latence USB-MIDI du breath controller

### Problème : Pas de son avec breath controller

**Symptômes :** CC2 reçu mais pas de son

**Solutions :**
1. Vérifier `CC2_ENABLED true` dans settings.h
2. Vérifier `CC2_SILENCE_THRESHOLD` (peut-être trop haut)
3. Vérifier debug : CC2 lissé dépasse-t-il le seuil ?

### Problème : Fallback velocity ne fonctionne pas

**Symptômes :** Pas de son si CC2 absent

**Solutions :**
1. Vérifier `CC2_TIMEOUT_MS` > 0
2. Vérifier velocity Note On > 0
3. Possible que CC2 soit encore "actif" (buffer non vide)

---

## 📚 Ressources

**Documentation associée :**
- [MIDI_CC_IMPLEMENTATION.md](MIDI_CC_IMPLEMENTATION.md) - Tous les CC implémentés
- [ARCHITECTURE.md](ARCHITECTURE.md) - Architecture globale du code
- [README_V3.md](README_V3.md) - Vue d'ensemble Servo Flute V3

**Commits associés :**
- `CC2 Breath Controller : Contrôle dynamique souffle en temps réel` (2026-02-04)
- `Suppression Pitch Bend : Logique incorrecte retirée` (2026-02-04)

**Matériel recommandé :**
- [Yamaha BC3A Breath Controller](https://www.yamaha.com/)
- [TEControl BBC2 Breath Controller](https://tecontrol.se/)
- [AKAI Professional EWI Series](https://www.akaipro.com/)

---

**Créé le :** 2026-02-04
**Version Servo Flute :** V3
**Statut :** ✅ Production
