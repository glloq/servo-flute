# TIMING ET ANTICIPATION - Servo Flute V3

## Problématique

Lorsqu'on envoie des événements MIDI avec des timestamps précis, on s'attend à ce que le **son soit produit exactement au timing MIDI**, pas avec un retard mécanique.

### Exemple du problème

Sans anticipation :
```
Note A reçue à t=0ms MIDI
  → Séquence démarre à t=0ms
  → Servos bougent pendant 100ms
  → Stabilisation 5ms
  → Son produit à t=105ms ❌ (retard de 105ms)

Note B reçue à t=150ms MIDI
  → Séquence démarre à t=150ms
  → Son produit à t=255ms ❌ (retard de 105ms)

Résultat : Les intervalles sont préservés (150ms) mais tout est décalé de 105ms !
```

## Solution : Anticipation automatique

### Principe

Pour chaque **NoteOn**, on **démarre la séquence EN AVANCE** pour compenser le délai mécanique total.

```
Délai mécanique total = SERVO_TO_SOLENOID_DELAY_MS
                      = 105ms (par défaut)
```

Ce délai unique englobe :
- Temps de déplacement des servos doigts (~100ms)
- Stabilisation mécanique (~5ms)

### Algorithme

```cpp
Pour un NoteOn avec timestamp MIDI = T :

  Temps de démarrage séquence = T - 105ms

  Si T < 105ms :
    → Démarrer immédiatement (impossible d'anticiper dans le passé)
  Sinon :
    → Démarrer à (T - 105ms)

  Son sera produit à T ✅
```

### Exemple corrigé

```
Note A : timestamp MIDI = 0ms
  → Cible son : t=0ms
  → Démarrage séquence : max(0, 0-105) = 0ms (impossible d'anticiper)
  → Servos 0-100ms
  → Stabilisation 100-105ms
  → Son à t=105ms ⚠️ (retard de 105ms pour la 1ère note uniquement)

Note B : timestamp MIDI = 150ms
  → Cible son : t=150ms
  → Démarrage séquence : 150-105 = 45ms ✅
  → Servos 45-145ms
  → Stabilisation 145-150ms
  → Son à t=150ms ✅ PILE au timing MIDI !

Note C : timestamp MIDI = 200ms
  → Cible son : t=200ms
  → Démarrage séquence : 200-105 = 95ms ✅
  → Servos 95-195ms
  → Stabilisation 195-200ms
  → Son à t=200ms ✅ PILE au timing MIDI !
```

## Implications

### ✅ Avantages

1. **Synchronisation MIDI précise** : Les notes sonnent exactement au timing prévu (sauf la 1ère)
2. **Intervalles exacts** : Les écarts entre notes sont respectés au millisecond près
3. **Pas de décalage cumulatif** : Le retard ne s'accumule pas sur une longue séquence

### ⚠️ Limites

1. **Première note toujours en retard** : La toute première note aura un retard de 105ms (impossible d'anticiper avant t=0)
2. **Notes rapides (<105ms)** : Si deux notes sont espacées de moins de 105ms :
   - La 2ème commencera pendant que la 1ère est encore en cours
   - Interruption de la 1ère note (pas encore jouée)
   - Seule la 2ème sera jouée

### Cas limites

#### Cas 1 : Notes très rapides (< 105ms d'écart)

```
Note A : t=0ms
Note B : t=50ms (écart de 50ms < 105ms)

Séquence :
  t=0ms   : Démarrage séquence A
  t=0-50ms: Servos en mouvement pour A
  t=50ms  : Note B arrive !
          → Démarrage séquence B (interruption de A)
          → A n'est JAMAIS jouée ❌
  t=50-150ms : Servos pour B
  t=155ms : Son B produit ✅

Résultat : Note A ignorée, seule B est jouée
```

**Solution** : Limiter le tempo MIDI à 60/(105ms) = 571 BPM en doubles croches (très rapide, rarement atteint)

#### Cas 2 : Première note d'une séquence

```
Note A : t=0ms (première note)

Impossible de démarrer à t=-105ms !

Séquence :
  t=0ms  : Démarrage immédiat
  t=105ms: Son produit

Retard inévitable de 105ms pour la 1ère note
```

**Solution** : Accepter ce retard OU ajouter un "lead-in" de 105ms au début de toute séquence MIDI

## Implémentation technique

### Code (NoteSequencer.cpp)

```cpp
void NoteSequencer::processNextEvent() {
  MidiEvent* event = _eventQueue.peek();
  if (event == nullptr) return;

  unsigned long eventAbsoluteTime = _playbackStartTime + event->timestamp;

  // Délai mécanique total
  const unsigned long MECHANICAL_DELAY = SERVO_TO_SOLENOID_DELAY_MS;

  // ANTICIPATION pour NoteOn
  unsigned long startTime;
  if (event->type == EVENT_NOTE_ON) {
    if (eventAbsoluteTime > MECHANICAL_DELAY) {
      startTime = eventAbsoluteTime - MECHANICAL_DELAY;  // Anticiper
    } else {
      startTime = 0;  // Première note, pas d'anticipation possible
    }
  } else {
    startTime = eventAbsoluteTime;  // NoteOff : timing exact
  }

  // Démarrer si le moment est venu
  if (millis() >= startTime) {
    // Traitement...
  }
}
```

### Messages de debug

Avec `DEBUG = 1`, le Serial affichera :

```
DEBUG: NoteSequencer - Début séquence note: 72 | Démarrage à t=0ms | Son prévu à t=0ms (dans 105ms)
DEBUG: NoteSequencer - 🎵 SON produit note 72 (vel: 80) | t=105ms | Cible: 0ms | Erreur: +105ms

DEBUG: NoteSequencer - Début séquence note 74 | Démarrage à t=45ms | Son prévu à t=150ms (dans 105ms)
DEBUG: NoteSequencer - 🎵 SON produit note 74 (vel: 64) | t=150ms | Cible: 150ms | Erreur: 0ms

DEBUG: NoteSequencer - Début séquence note 76 | Démarrage à t=95ms | Son prévu à t=200ms (dans 105ms)
DEBUG: NoteSequencer - 🎵 SON produit note 76 (vel: 100) | t=200ms | Cible: 200ms | Erreur: 0ms
```

**Interprétation** :
- Note 72 : Erreur +105ms (retard première note)
- Note 74 : Erreur 0ms ✅ (anticipation réussie)
- Note 76 : Erreur 0ms ✅ (anticipation réussie)

## Réglage fin

### Ajuster le délai total

Si les servos sont plus rapides/lents que prévu :

```cpp
// settings.h
#define SERVO_TO_SOLENOID_DELAY_MS  90   // Réduire si servos plus rapides
// OU
#define SERVO_TO_SOLENOID_DELAY_MS  120  // Augmenter si servos plus lents
```

### Mesurer le délai réel

Observer les messages debug "Erreur:" :
- Erreur négative (-10ms) : Le son arrive AVANT le timing prévu → Augmenter les délais
- Erreur positive (+10ms) : Le son arrive APRÈS → Réduire les délais
- Erreur ~0ms : Parfait ✅

## Conclusion

L'anticipation automatique permet de **respecter le timing MIDI original** avec une précision de quelques millisecondes, transformant le servo-flute en un instrument synchronisé et prévisible.

**Seule exception** : La toute première note aura toujours un retard égal au délai mécanique (105ms par défaut).
