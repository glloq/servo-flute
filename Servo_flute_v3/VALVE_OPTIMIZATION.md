# OPTIMISATION DE LA VALVE - Servo Flute V3

## Problématique

Le solénoïde qui contrôle la valve air a plusieurs contraintes :
- **Usure mécanique** : Chaque ouverture/fermeture use le mécanisme
- **Consommation électrique** : 200-500mA par activation
- **Bruit** : Claquement audible à chaque activation
- **Latence** : ~5-10ms pour ouvrir/fermer

Pour des notes rapides consécutives (ex: trilles, passages rapides), ouvrir/fermer la valve entre chaque note est inefficace et détériore la qualité musicale.

## Solution : Valve intelligente

### Principe

**Si deux notes sont suffisamment proches dans le temps, on garde la valve ouverte et on change seulement les doigts.**

### Paramètre de configuration

```cpp
// settings.h
#define MIN_NOTE_INTERVAL_FOR_VALVE_CLOSE_MS  50
```

**Logique** :
- Si intervalle entre notes **< 50ms** : Valve reste **OUVERTE**, débit réduit au minimum
- Si intervalle entre notes **≥ 50ms** : Valve se **FERME** normalement

### Algorithme

```
Lors de stopCurrentNote() :

1. Regarder la prochaine note dans la queue EventQueue

2. Si aucune note suivante :
   → Fermer la valve

3. Si note suivante existe :
   a. Calculer temps jusqu'à la prochaine note
   b. Si temps < MIN_NOTE_INTERVAL_FOR_VALVE_CLOSE_MS :
      → Garder valve OUVERTE
      → Réduire débit au minimum (velocity=1)
      → Les doigts vont se repositionner pendant que l'air circule
   c. Sinon :
      → Fermer valve normalement
```

## Exemple concret

### Cas 1 : Notes espacées (≥ 50ms) - Valve fermée

```
Note A (t=0ms, durée=200ms)
Note B (t=300ms, durée=200ms)

Intervalle = 300ms - 200ms = 100ms ≥ 50ms

Séquence :
  t=0ms    : Valve OUVERTE, note A joue
  t=200ms  : NoteOff A → Valve FERMÉE (intervalle > 50ms)
  t=300ms  : Valve OUVERTE, note B joue
  t=500ms  : NoteOff B → Valve FERMÉE

Résultat :
  - Silence clair entre les notes ✅
  - 4 activations valve (2 ouvertures + 2 fermetures)
```

### Cas 2 : Notes rapides (< 50ms) - Valve ouverte

```
Note A (t=0ms, durée=100ms)
Note B (t=120ms, durée=100ms)

Intervalle = 120ms - 100ms = 20ms < 50ms

Séquence :
  t=0ms    : Valve OUVERTE, note A joue
  t=100ms  : NoteOff A → Valve RESTE OUVERTE (intervalle < 50ms)
           → Débit réduit au minimum
  t=100-120ms : Servos doigts se repositionnent (air circule légèrement)
  t=120ms  : Valve déjà ouverte, débit augmenté, note B joue immédiatement
  t=220ms  : NoteOff B → Valve FERMÉE

Résultat :
  - Transition fluide entre notes ✅
  - Économie 1 fermeture + 1 ouverture = 50% moins de cycles
  - Réactivité améliorée (pas de latence solénoïde)
  - Léger souffle entre notes (air minimal)
```

### Cas 3 : Trille rapide (5 notes)

```
Notes A-B-A-B-A espacées de 30ms

Séquence :
  t=0ms   : Valve OUVERTE, note A
  t=30ms  : NoteOff A → Valve RESTE OUVERTE (30ms < 50ms)
  t=30ms  : Note B démarre, valve déjà ouverte
  t=60ms  : NoteOff B → Valve RESTE OUVERTE
  t=60ms  : Note A démarre
  t=90ms  : NoteOff A → Valve RESTE OUVERTE
  t=90ms  : Note B démarre
  t=120ms : NoteOff B → Valve RESTE OUVERTE
  t=120ms : Note A démarre
  t=150ms : NoteOff A → Valve FERMÉE (pas de note suivante)

Résultat :
  - 1 seule ouverture + 1 seule fermeture
  - Économie : 8 cycles valve évités !
  - Trille ultra-fluide
```

## Avantages

| Aspect | Valve toujours fermée | Valve intelligente |
|--------|----------------------|-------------------|
| **Usure mécanique** | Élevée (2n cycles) | Réduite (2 à 4 cycles) |
| **Consommation** | Élevée | Réduite |
| **Fluidité notes rapides** | Moyenne | Excellente |
| **Silence entre notes** | Parfait | Léger souffle si < 50ms |
| **Latence** | 5-10ms par activation | Aucune si valve ouverte |

## Réglage du seuil

### Valeurs typiques

```cpp
#define MIN_NOTE_INTERVAL_FOR_VALVE_CLOSE_MS  30  // Très agressif
#define MIN_NOTE_INTERVAL_FOR_VALVE_CLOSE_MS  50  // Équilibré (défaut)
#define MIN_NOTE_INTERVAL_FOR_VALVE_CLOSE_MS  100 // Conservateur
```

### Comment choisir ?

**Seuil bas (30ms)** :
- ✅ Silence meilleur entre notes
- ✅ Moins de souffle parasite
- ❌ Plus d'usure valve
- ❌ Moins fluide sur passages rapides

**Seuil élevé (100ms)** :
- ✅ Économie maximale valve
- ✅ Fluidité maximale
- ❌ Souffle audible entre notes
- ❌ Séparation notes moins claire

**Seuil médian (50ms)** - RECOMMANDÉ :
- ⚖️ Bon compromis
- Notes < 50ms espacées : fluides (trilles, ornements)
- Notes > 50ms espacées : silence clair (mélodies)

### Test empirique

1. Jouer une gamme lente (notes espacées de 200ms+)
   → Doit avoir des silences clairs entre notes

2. Jouer un trille rapide (30ms entre notes)
   → Doit être fluide sans claquements valve

3. Ajuster si nécessaire selon préférence musicale

## Messages de debug

Avec `DEBUG = 1`, observer :

```
DEBUG: NoteSequencer - Arrêt note: 72 (valve fermée)
```
→ Notes bien espacées, comportement normal

```
DEBUG: NoteSequencer - Valve GARDÉE ouverte (note suivante dans 35ms)
DEBUG: NoteSequencer - Arrêt note: 74 (valve OUVERTE, débit minimal)
```
→ Notes rapides, optimisation active

## Limitation

**La valve reste ouverte pendant le repositionnement des doigts**, ce qui produit un léger souffle avec la combinaison de doigtés intermédiaire.

**Solutions possibles** :
1. Accepter ce léger souffle (souvent inaudible à vitesse normale)
2. Réduire encore plus le débit minimal (ajuster dans le code)
3. Diminuer `MIN_NOTE_INTERVAL_FOR_VALVE_CLOSE_MS` si gênant

## Statistiques attendues

Pour une mélodie typique (100 notes) :
- **Sans optimisation** : ~200 cycles valve (2 par note)
- **Avec optimisation** : ~50-100 cycles valve (50% économie)
- **Pour un trille de 10 notes** : 2 cycles au lieu de 20 (90% économie)

Durée de vie valve augmentée d'un facteur 2-4 selon répertoire joué.
