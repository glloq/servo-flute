# Documentation Servo Flute V3

Ce dossier contient toute la documentation technique du projet Servo Flute V3.

---

## 📚 Index de la documentation

### Configuration et guides
- **[README_V3.md](README_V3.md)** - Vue d'ensemble architecture V3
- **[CONFIGURATION_GUIDE.md](CONFIGURATION_GUIDE.md)** - Guide configuration générale
- **[INSTRUMENTS_GUIDE.md](INSTRUMENTS_GUIDE.md)** - Guide multi-instruments

### MIDI Control Change (CC)
- **[MIDI_CC_IMPLEMENTATION.md](MIDI_CC_IMPLEMENTATION.md)** - Documentation complète CC + historique correctifs
- **[CC7_VIBRATO_FIX.md](CC7_VIBRATO_FIX.md)** - Fix CC7/vibrato : respect bornes note
- **[CC_AUDIT_REPORT.md](CC_AUDIT_REPORT.md)** - Audit 46 points ✅ 4 critiques RÉSOLUS

### Optimisations système
- **[SOLENOID_PWM.md](SOLENOID_PWM.md)** - Gestion PWM solénoïde (deux phases)
- **[TIMING_ANTICIPATION.md](TIMING_ANTICIPATION.md)** - Délai fixe 105ms pour positionnement servos
- **[VALVE_OPTIMIZATION.md](VALVE_OPTIMIZATION.md)** - Optimisation ouverture/fermeture valve

---

## 🔍 Ordre de lecture recommandé

### Pour débuter
1. **README_V3.md** - Comprendre architecture globale
2. **CONFIGURATION_GUIDE.md** - Apprendre à configurer le système
3. **INSTRUMENTS_GUIDE.md** - Adapter à différents instruments

### Pour utiliser MIDI CC
1. **MIDI_CC_IMPLEMENTATION.md** - Documentation complète + historique correctifs
2. **CC_AUDIT_REPORT.md** - Audit et optimisations identifiées

### Pour optimiser/débugger
1. **CC_AUDIT_REPORT.md** - Points d'amélioration identifiés
2. **TIMING_ANTICIPATION.md** - Comprendre délai mécanique 105ms
3. **VALVE_OPTIMIZATION.md** - Optimisations valve
4. **SOLENOID_PWM.md** - Gestion avancée solénoïde

---

## 📊 Historique des correctifs

### 2026-01-26 - Fix CC7 et vibrato : Respect bornes note
- ✅ CC7 module maintenant dans [minAngle, modulatedAngle] (au lieu de multiplier globalement)
- ✅ Vibrato limité aux bornes de la note en cours
- ✅ Garantie : servo toujours dans intervalle [airflowMinPercent, airflowMaxPercent]
- ✅ Protection matériel : CC7=0 ou vibrato fort ne peuvent plus forcer servo hors limites

### 2026-01-26 - Consolidation documentation
- 🗑️ Suppression CC11_LOGIC_UPDATE.md et CC_CRITICAL_FIXES.md (redondants)
- ✅ Historique intégré dans MIDI_CC_IMPLEMENTATION.md
- ✅ Documentation simplifiée : 9 docs au lieu de 11
- ✅ Pas de perte d'information (tout consolidé)

### 2026-01-26 - Nettoyage documentation
- 🗑️ Suppression 3 documents préliminaires obsolètes (remplacés par MIDI_CC_IMPLEMENTATION.md)
- ✅ Mise à jour CC_AUDIT_REPORT.md : 4 problèmes critiques marqués comme résolus

### 2026-01-26 - Correctifs critiques CC
- ✅ Vibrato fonctionnel (update continu)
- ✅ Optimisation sin() LUT (gain 25x performance)
- ✅ Validation CC ranges (sécurité)
- ✅ Fix overflow millis()

### 2026-01-25 - Fix logique CC11
- ✅ CC11 respecte airflowMinPercent/MaxPercent
- ✅ Distinction claire CC7 (volume) vs CC11 (expression)

### 2026-01-25 - Implémentation CC
- ✅ CC 1 (Modulation/Vibrato)
- ✅ CC 7 (Volume)
- ✅ CC 11 (Expression)
- ✅ CC 120 (All Sound Off)

### 2026-01-24 - Fix airflow physique
- ✅ Plus de trous fermés = PLUS d'air (logique corrigée)

---

## 🎯 Fichiers par catégorie

### Documentation de référence
- MIDI_CC_IMPLEMENTATION.md (utilisation + historique)
- CC_AUDIT_REPORT.md (audit + résolutions)

### Optimisations système
- SOLENOID_PWM.md
- TIMING_ANTICIPATION.md
- VALVE_OPTIMIZATION.md

### Guides utilisateur
- README_V3.md
- CONFIGURATION_GUIDE.md
- INSTRUMENTS_GUIDE.md

---

**Version :** V3
**Dernière mise à jour :** 2026-01-26
