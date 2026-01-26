# Mise à jour logique CC11 (Expression)

## 📋 Changement apporté

**Date :** 2026-01-25
**Version :** V3
**Raison :** CC11 doit respecter les bornes airflowMinPercent/MaxPercent de chaque note

---

## ❌ Comportement AVANT (problématique)

### Formule ancienne
```cpp
baseAngle = map(velocity, 1, 127, minAngle, maxAngle);
finalAngle = baseAngle × (CC7/127) × (CC11/127);
```

### Problème identifié

```
Note C6:
- airflowMinPercent = 20% → minAngle = 68°
- airflowMaxPercent = 75% → maxAngle = 90°

Velocity 127 → baseAngle = 90°
CC7 = 127, CC11 = 50

Calcul:
  angle = 90 × 1.0 × (50/127)
  angle = 35.4°

❌ PROBLÈME: 35.4° < 68° (en dessous du minimum de la note!)
```

**Conséquence :** CC11 pouvait faire descendre l'airflow **en dessous** du minimum physique requis par la note, ce qui n'a pas de sens musicalement.

---

## ✅ Comportement APRÈS (corrigé)

### Formule nouvelle
```cpp
// 1. Velocity définit angle de base
baseAngle = map(velocity, 1, 127, minAngle, maxAngle);

// 2. CC11 module DANS [minAngle, baseAngle]
expressionFactor = CC11 / 127;
modulatedAngle = minAngle + (baseAngle - minAngle) × expressionFactor;

// 3. CC7 multiplicateur global (appliqué après)
finalAngle = modulatedAngle × (CC7/127);
```

### Comportement correct

```
Note C6:
- minAngle = 68°, maxAngle = 90°

Velocity 127 → baseAngle = 90°

CC11 = 127 → modulatedAngle = 68 + (90-68)×1.0 = 90° ✓
CC11 = 64  → modulatedAngle = 68 + (90-68)×0.5 = 79° ✓
CC11 = 0   → modulatedAngle = 68 + (90-68)×0.0 = 68° ✓

Puis CC7 = 127 → finalAngle = modulatedAngle (inchangé)

✓ CC11 reste TOUJOURS dans [68°, 90°]
```

---

## 🎯 Différence CC7 vs CC11

### CC11 (Expression) - Respecte bornes note
- **Rôle :** Expression musicale DANS la plage de la note
- **Plage :** `[airflowMinPercent, baseAngle]`
- **Usage :** Crescendo/diminuendo naturel
- **Limite :** Ne peut PAS descendre sous `minAngle` de la note

### CC7 (Volume) - Multiplicateur global
- **Rôle :** Volume "master" général
- **Plage :** Peut descendre sous `minAngle` (c'est un volume global)
- **Usage :** Ajustement volume concert/répétition
- **Limite :** Aucune (multiplicateur libre)

---

## 📊 Exemples comparatifs

### Exemple 1 : Crescendo naturel

**Configuration :**
- Note C6: plage [68°, 90°]
- Velocity 127 → baseAngle = 90°
- CC7 = 127 (volume normal)

**CC11 progressif :**
```
CC11 = 0   → modulatedAngle = 68° → finalAngle = 68° (pianissimo)
CC11 = 32  → modulatedAngle = 73.5° → finalAngle = 73.5° (piano)
CC11 = 64  → modulatedAngle = 79° → finalAngle = 79° (mezzo-forte)
CC11 = 96  → modulatedAngle = 84.5° → finalAngle = 84.5° (forte)
CC11 = 127 → modulatedAngle = 90° → finalAngle = 90° (fortissimo)
```

**✓ Avantage :** Crescendo respecte la physique de la note (toujours ≥ 68°)

---

### Exemple 2 : Volume global réduit

**Configuration :**
- Note C6: plage [68°, 90°]
- Velocity 127 → baseAngle = 90°
- CC11 = 127 (pleine expression)

**CC7 progressif :**
```
CC7 = 127 → finalAngle = 90° (volume normal)
CC7 = 96  → finalAngle = 68° (volume réduit 75%)
CC7 = 64  → finalAngle = 45° (volume réduit 50%) ← Peut descendre sous 68°!
CC7 = 32  → finalAngle = 22.5° (volume très faible)
```

**✓ Comportement attendu :** CC7 est un contrôle "master" qui peut réduire sous les bornes

---

### Exemple 3 : Combinaison CC7 + CC11

**Configuration :**
- Note C6: plage [68°, 90°]
- Velocity 100 → baseAngle = 86°
- CC7 = 80 (volume réduit à 63%)

**CC11 progressif :**
```
CC11 = 0   → modulatedAngle = 68° → finalAngle = 68×0.63 = 43° (pianissimo + volume réduit)
CC11 = 64  → modulatedAngle = 77° → finalAngle = 77×0.63 = 48.5° (mezzo + volume réduit)
CC11 = 127 → modulatedAngle = 86° → finalAngle = 86×0.63 = 54° (fortissimo + volume réduit)
```

**✓ CC11 module l'expression, CC7 ajuste le volume global**

---

## 🔧 Fichiers modifiés

### AirflowController.cpp - setAirflowForNote()

**AVANT :**
```cpp
// Calcul angle de base
uint16_t angle = map(velocity, 1, 127, minAngle, maxAngle);

// Appliquer CC7
float finalAngle = angle × (CC7/127);

// Appliquer CC11
finalAngle = finalAngle × (CC11/127); // ❌ Problème ici
```

**APRÈS :**
```cpp
// 1. Calcul angle de base
uint16_t baseAngle = map(velocity, 1, 127, minAngle, maxAngle);

// 2. CC11 module DANS [minAngle, baseAngle]
float expressionFactor = CC11 / 127.0;
float modulatedAngle = minAngle + (baseAngle - minAngle) × expressionFactor;

// 3. CC7 multiplicateur global
float finalAngle = modulatedAngle × (CC7/127);
```

### MIDI_CC_IMPLEMENTATION.md

Sections mises à jour :
- Description CC11 (ajout mention "DANS les bornes")
- Formule complète de calcul (nouvelle formule détaillée)
- Exemple concret (nouveau calcul avec comparaison CC11)
- Scénarios d'utilisation (crescendo respecte bornes)
- **Nouvelle section :** "🎯 Différence CC7 vs CC11"

---

## 🧪 Tests de validation

### Test 1 : CC11 respecte minAngle
```
Note C6 [68°, 90°], Velocity 127, CC7 = 127

CC11 = 0   → finalAngle devrait être ≥ 68° ✓
CC11 = 127 → finalAngle devrait être ≤ 90° ✓

Vérifier dans DEBUG:
"Range: 68°-90° | BaseAngle: 90° | CC11: 0 → 68°"
```

### Test 2 : CC11 avec différentes velocities
```
Note C6 [68°, 90°], CC7 = 127, CC11 = 64

Velocity 50  → baseAngle ≈ 77° → modulatedAngle ≈ 72.5°
Velocity 100 → baseAngle ≈ 86° → modulatedAngle ≈ 77°
Velocity 127 → baseAngle = 90° → modulatedAngle = 79°

✓ CC11 reste toujours dans [68°, baseAngle]
```

### Test 3 : Combinaison CC7 + CC11
```
Note C6 [68°, 90°], Velocity 127

CC11 = 127, CC7 = 64 → finalAngle ≈ 45° (volume réduit) ✓
CC11 = 0,   CC7 = 64 → finalAngle ≈ 34° (pianissimo + volume réduit) ✓

✓ CC7 peut réduire sous minAngle (comportement attendu)
```

---

## 📚 Impact sur utilisateurs

### Comportement musical amélioré

**Avant :**
- CC11 pouvait créer des notes "impossibles" physiquement
- Expression pouvait être incohérente avec les capacités de la note
- Confusion entre volume (CC7) et expression (CC11)

**Après :**
- CC11 crée un crescendo/diminuendo **naturel** dans la plage de la note
- Expression respecte les limites physiques (min/max airflow)
- Distinction claire : CC11 = expression, CC7 = volume

### Rétrocompatibilité

**Par défaut (CC7=127, CC11=127) :**
- Comportement identique à avant
- Velocity seule contrôle airflow
- Aucun changement pour utilisateurs n'utilisant pas les CC

**Avec CC11 < 127 :**
- Nouveau comportement (respecte bornes)
- Plus musical et cohérent
- Peut nécessiter ajustement automation DAW si utilisé avant

---

## ✅ Résumé

**Changement principal :**
```
CC11 ne multiplie plus librement → CC11 module DANS [minAngle, baseAngle]
```

**Avantages :**
- ✓ Respect des bornes physiques de chaque note
- ✓ Expression musicale naturelle (crescendo/diminuendo)
- ✓ Distinction claire CC7 (volume) vs CC11 (expression)
- ✓ Cohérent avec la configuration airflowMinPercent/MaxPercent

**Fichiers modifiés :**
- `AirflowController.cpp` (~20 lignes modifiées)
- `MIDI_CC_IMPLEMENTATION.md` (documentation complète mise à jour)

**Tests recommandés :**
- Crescendo naturel avec CC11
- Vérifier bornes respectées dans DEBUG
- Combiner CC7 + CC11

---

**Documentation créée le :** 2026-01-25
**Version Servo Flute :** V3
**Commit :** Prochaine mise à jour
