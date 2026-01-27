# Fix CC7 et vibrato : Respect bornes note

**Date :** 2026-01-26
**Problème :** CC7 et vibrato pouvaient sortir de l'intervalle [airflowMinPercent, airflowMaxPercent] de la note

---

## 🔴 Problème identifié

### CC7 (Volume) - Multiplication globale

**Ancien code :**
```cpp
// CC7 multipliait globalement
float finalAngle = modulatedAngle * (_ccVolume / 127.0);
```

**Bug :**
```
Note : minAngle = 20°, maxAngle = 80°
Velocity 127 → baseAngle = 80°
CC11 = 127 → modulatedAngle = 80°
CC7 = 16 → finalAngle = 80° × 0.125 = 10° ❌ EN DESSOUS de minAngle (20°) !
```

### Vibrato (CC1) - Bornes servo globales

**Ancien code :**
```cpp
// Vibrato limité avec bornes servo globales
if (finalAngle < SERVO_AIRFLOW_MIN) finalAngle = SERVO_AIRFLOW_MIN;
if (finalAngle > SERVO_AIRFLOW_MAX) finalAngle = SERVO_AIRFLOW_MAX;
```

**Bug :**
```
Note : minAngle = 60°, maxAngle = 80°
Base = 70°
Vibrato ±8° → peut aller à 62° (OK) ou 78° (OK)

MAIS avec vibrato très fort :
Base = 70°, Vibrato ±20° → peut aller à 50° ❌ EN DESSOUS de minAngle (60°)
```

---

## ✅ Solution implémentée

### 1. CC7 module DANS l'intervalle note

**Nouveau code :**
```cpp
// CC7 (Volume) module DANS [minAngle, modulatedAngle]
float volumeFactor = _ccVolume / 127.0;
float finalAngleWithoutVibrato = minAngle + (modulatedAngle - minAngle) * volumeFactor;
```

**Résultat :**
```
Note : minAngle = 20°, maxAngle = 80°
CC11 = 127 → modulatedAngle = 80°

CC7 = 127 → finalAngle = 20° + (80° - 20°) × 1.0 = 80° ✅
CC7 = 64  → finalAngle = 20° + (80° - 20°) × 0.5 = 50° ✅
CC7 = 0   → finalAngle = 20° + (80° - 20°) × 0.0 = 20° ✅ (jamais en dessous!)
```

### 2. Vibrato limité aux bornes note

**Ajout variables :**
```cpp
// AirflowController.h
uint16_t _currentMinAngle;  // Angle min de la note en cours
uint16_t _currentMaxAngle;  // Angle max de la note en cours
```

**Stockage dans setAirflowForNote() :**
```cpp
// Stocker les bornes pour limiter le vibrato ultérieurement
_currentMinAngle = minAngle;
_currentMaxAngle = maxAngle;
```

**Limite dans update() :**
```cpp
// Vibrato limité aux bornes de la NOTE EN COURS
if (finalAngle < (int16_t)_currentMinAngle) finalAngle = _currentMinAngle;
if (finalAngle > (int16_t)_currentMaxAngle) finalAngle = _currentMaxAngle;
```

---

## 🎯 Nouvelle logique complète

```cpp
// 1. VELOCITY → baseAngle dans [minAngle, maxAngle]
baseAngle = minAngle + (maxAngle - minAngle) × (velocity / 127.0)

// 2. CC11 (Expression) → module dans [minAngle, baseAngle]
modulatedAngle = minAngle + (baseAngle - minAngle) × (CC11 / 127.0)

// 3. CC7 (Volume) → module dans [minAngle, modulatedAngle]
finalAngle = minAngle + (modulatedAngle - minAngle) × (CC7 / 127.0)

// 4. CC1 (Vibrato) → oscille autour finalAngle, limité à [minAngle, maxAngle]
vibratoOffset = sin(...) × amplitude
finalWithVibrato = clamp(finalAngle + vibratoOffset, minAngle, maxAngle)
```

---

## 📊 Garanties

**Avec cette correction :**

✅ **CC7 = 0** → servo à minAngle (silence minimum autorisé par la note)
✅ **CC7 = 127** → servo à modulatedAngle (pleine intensité selon velocity + CC11)
✅ **CC7 ne peut JAMAIS** descendre sous minAngle
✅ **CC7 ne peut JAMAIS** dépasser maxAngle

✅ **Vibrato oscillation** toujours dans [minAngle, maxAngle]
✅ **Vibrato ne peut JAMAIS** sortir des bornes de la note

---

## 🧪 Tests de validation

### Test 1 : CC7 bas ne descend pas sous minAngle

```
Note : min 20%, max 80% (minAngle = 92°, maxAngle = 152°)
Velocity 127 → baseAngle = 152°
CC11 = 127 → modulatedAngle = 152°

CC7 = 127 → finalAngle = 152° ✅
CC7 = 64  → finalAngle = 122° ✅ (au milieu de [92, 152])
CC7 = 0   → finalAngle = 92° ✅ (PILE au minAngle, jamais en dessous)
```

### Test 2 : Vibrato reste dans bornes

```
Note : min 50%, max 70% (minAngle = 122°, maxAngle = 140°)
BaseAngle = 131° (au milieu)
Vibrato ±8° → oscillation entre 123° et 139° ✅ (dans [122, 140])

Vibrato ±20° (très fort) :
- Min vibrato : 131° - 20° = 111° → CLAMPÉ à 122° ✅
- Max vibrato : 131° + 20° = 151° → CLAMPÉ à 140° ✅
```

---

## 📝 Fichiers modifiés

### AirflowController.h
- Ajout `_currentMinAngle` et `_currentMaxAngle`

### AirflowController.cpp

**Constructeur :**
```cpp
_currentMinAngle(SERVO_AIRFLOW_MIN), _currentMaxAngle(SERVO_AIRFLOW_MAX)
```

**setAirflowForNote() :**
- Stockage `_currentMinAngle` et `_currentMaxAngle`
- CC7 module dans `[minAngle, modulatedAngle]`

**update() :**
- Vibrato limité avec `_currentMinAngle` et `_currentMaxAngle`

---

## ✅ Impact

**Correctif critique :** Sans ce fix, CC7 bas ou vibrato fort pouvaient endommager le matériel en forçant le servo hors de sa plage sûre pour la note.

**Sécurité :** Garantie absolue que le servo reste dans l'intervalle défini par la note.

**Musical :** CC7 et vibrato fonctionnent correctement tout en respectant les contraintes physiques de chaque note.

---

**Version :** V3
**Statut :** ✅ Implémenté et prêt pour tests
