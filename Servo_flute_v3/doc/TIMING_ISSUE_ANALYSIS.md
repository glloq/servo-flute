# Analyse problème timing - MIDI temps réel vs séquences

**Date :** 2026-01-26
**Problème rapporté :** "Les intervalles sont préservés (150ms) et tout est décalé de 105ms"

---

## 🔴 Problème identifié

L'anticipation décrite dans TIMING_ANTICIPATION.md **ne fonctionne PAS pour MIDI en temps réel**, seulement pour séquences pré-enregistrées.

### Comportement actuel avec MIDI temps réel

Quand on joue au clavier MIDI :

```
t=1000ms : Note A arrive via USB MIDI
         → InstrumentManager.noteOn() appelé
         → EventQueue.enqueue(..., millis()=1000)
         → _referenceTime = 1000, timestamp relatif = 0

t=1005ms : NoteSequencer.processNextEvent() traite Note A
         → _playbackStartTime = millis() = 1005
         → eventAbsoluteTime = 1005 + 0 = 1005
         → startTime = 1005 - 105 = 900
         → millis()(1005) >= 900 ? OUI → Démarre IMMÉDIATEMENT
         → Son produit à 1005 + 105 = 1110ms

Résultat : Retard de ~110ms (pas d'anticipation possible)

t=1150ms : Note B arrive
         → Queue vide depuis fin de Note A
         → EventQueue ligne 49-52 a RESET _referenceTime !
         → Nouvelle _referenceTime = 1150, timestamp relatif = 0

t=1155ms : Traitement Note B
         → _playbackStartTime = 1155 (nouveau playback !)
         → eventAbsoluteTime = 1155 + 0 = 1155
         → startTime = 1155 - 105 = 1050
         → millis()(1155) >= 1050 ? OUI → Démarre IMMÉDIATEMENT
         → Son produit à 1155 + 105 = 1260ms

Intervalle réel entre sons : 1260 - 1110 = 150ms ✅
Décalage absolu : Chaque note retardée de ~110ms ✅
```

**C'est exactement le problème rapporté !**

---

## 🎯 Cause racine

### Problème 1 : Reset de _referenceTime

**EventQueue.cpp ligne 48-52** :
```cpp
// Si la queue devient vide, reset la référence temporelle
if (_count == 0) {
  _hasReference = false;
  _referenceTime = 0;
}
```

**Conséquence :** Chaque note isolée (MIDI temps réel) devient un "nouveau premier événement" → Impossible d'anticiper.

### Problème 2 : Design pour séquences pré-enregistrées

Le système actuel suppose :
- ✅ Événements ajoutés AVANT lecture (séquence MIDI complète en mémoire)
- ✅ Timestamps relatifs connus à l'avance (0ms, 150ms, 300ms, ...)
- ✅ _playbackStartTime initialisé au début de la lecture
- ✅ Anticipation possible car événements futurs connus

**MIDI temps réel :**
- ❌ Événements arrivent UN PAR UN au fil de l'eau
- ❌ Timestamp = "maintenant" (pas de notion de futur)
- ❌ Impossible d'anticiper ce qu'on ne connait pas encore

---

## 💡 Solutions possibles

### Solution 1 : Mode "MIDI temps réel" (RECOMMANDÉ)

**Principe :** Pour MIDI en temps réel, **accepter le retard de 105ms** comme incompressible.

**Changements :**

1. **NoteSequencer.cpp** : Détecter mode temps réel
```cpp
void NoteSequencer::processNextEvent() {
  MidiEvent* event = _eventQueue.peek();
  if (event == nullptr) return;

  // MODE TEMPS RÉEL : timestamp 0 = jouer immédiatement
  if (event->timestamp == 0) {
    // Pas d'anticipation possible
    if (millis() >= _playbackStartTime) {
      // Démarrer immédiatement la séquence
      if (event->type == EVENT_NOTE_ON) {
        if (_currentState != STATE_IDLE) stopCurrentNote();
        startNoteSequence(event->midiNote, event->velocity, millis() + SERVO_TO_SOLENOID_DELAY_MS);
        _eventQueue.dequeue();
      }
    }
    return;
  }

  // MODE SÉQUENCE PRÉ-ENREGISTRÉE : logique actuelle avec anticipation
  // ... (code actuel)
}
```

2. **EventQueue.cpp** : Ne PAS reset _referenceTime pour temps réel
```cpp
void EventQueue::dequeue() {
  if (isEmpty()) return;

  _tail = (_tail + 1) % _capacity;
  _count--;

  // NE PLUS reset automatiquement pour permettre continuité temporelle
  // La référence est reset seulement par clear() explicite
}
```

3. **InstrumentManager.cpp** : Mode temps réel = timestamp 0
```cpp
void InstrumentManager::noteOn(byte midiNote, byte velocity) {
  if (!isNotePlayable(midiNote)) return;

  // MIDI temps réel : timestamp = 0 (immédiat, pas de notion de futur)
  bool success = _eventQueue.enqueue(EVENT_NOTE_ON, midiNote, velocity, 0);

  // ... reste du code
}
```

**Résultat :**
- ✅ Chaque note MIDI temps réel démarre immédiatement
- ✅ Son produit après 105ms (retard incompressible)
- ✅ Intervalles entre notes préservés
- ⚠️ Retard absolu de 105ms accepté (inévitable pour MIDI temps réel)

---

### Solution 2 : Buffer d'anticipation (COMPLEXE)

**Principe :** Introduire un délai artificiel de 105ms pour permettre anticipation.

**Fonctionnement :**
```
t=0    : Note A arrive → Mise en buffer
t=105ms: Note A démarre (anticipation possible si Note B déjà reçue)
         Si Note B reçue entre t=0 et t=105, on peut anticiper

t=150  : Note B arrive → Mise en buffer
t=150  : Note A toujours en cours, Note B déjà connue
         → Anticipation de Note B possible !
```

**Inconvénients :**
- ❌ Latence artificielle de 105ms ajoutée
- ❌ Complexité accrue
- ❌ L'utilisateur ressent un décalage entre action et son

**Non recommandé** pour usage live.

---

### Solution 3 : Servos plus rapides (MATÉRIEL)

**Principe :** Réduire SERVO_TO_SOLENOID_DELAY_MS de 105ms → 50ms

**Changements :**
```cpp
// settings.h
#define SERVO_TO_SOLENOID_DELAY_MS  50  // Au lieu de 105
```

**Nécessite :**
- Servos haute vitesse (0.08s/60° au lieu de 0.17s/60°)
- Surcoût matériel

**Résultat :**
- ✅ Retard réduit de moitié (105ms → 50ms)
- ⚠️ Ne résout pas le problème fondamental
- 💰 Coût supplémentaire

---

## ✅ Recommandation

**Adopter Solution 1 : Mode MIDI temps réel**

**Raisons :**
1. **Réaliste** : On ne peut pas anticiper ce qu'on ne connait pas
2. **Simple** : Modifications minimales du code
3. **Honnête** : Accepter le retard incompressible de 105ms
4. **Fonctionnel** : Intervalles préservés, timing relatif correct

**Alternative future :**
- Pour séquences pré-enregistrées (fichiers MIDI), garder logique actuelle avec anticipation
- Implémenter détection automatique : timestamp==0 → temps réel, timestamp>0 → séquence

---

## 📝 Documentation à mettre à jour

**TIMING_ANTICIPATION.md** doit être clarifié :

**Section à ajouter :**
```markdown
## Limitation : MIDI temps réel

L'anticipation fonctionne UNIQUEMENT pour séquences pré-enregistrées.

Pour MIDI en temps réel (clavier, DAW live) :
- Chaque note arrive "maintenant" (pas de timestamps futurs)
- Impossible d'anticiper → Retard de 105ms inévitable
- Intervalles entre notes préservés ✅
- Timing absolu décalé de 105ms ✅ (acceptable)

Pour éliminer le retard :
1. Utiliser servos plus rapides (matériel)
2. OU accepter le retard comme latence naturelle de l'instrument
```

---

## 🧪 Tests de validation

### Test actuel (CONFIRME le problème)
```
Envoyer : Note C (60) à t=0ms
Observer : Son à ~110ms (retard)

Envoyer : Note D (62) à t=150ms
Observer : Son à ~260ms (retard)

Intervalle : 260-110 = 150ms ✅ (préservé)
Décalage : +110ms sur chaque note ❌
```

### Test après correction (Solution 1)
```
Mode temps réel détecté (timestamp==0)
Envoyer : Note C (60) → Démarre immédiatement
Observer : Son à 105ms (retard incompressible)

Envoyer : Note D (62) 150ms plus tard
Observer : Son à ~255ms

Intervalle : 255-105 = 150ms ✅
Décalage : Accepté comme latence naturelle ✅
```

---

## 📊 Comparaison solutions

| Solution | Latence | Complexité | Coût | Anticipation |
|----------|---------|------------|------|--------------|
| 1. Mode temps réel | 105ms | Faible | Aucun | Non (impossible) |
| 2. Buffer 105ms | 210ms | Élevée | Aucun | Oui |
| 3. Servos rapides | 50ms | Faible | $$$ | Non |

**Conclusion :** Solution 1 = meilleur compromis

---

**Statut :** Analyse terminée, implémentation de la correction recommandée à suivre.
