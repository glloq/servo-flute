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

## 🚀 Démarrage Rapide

### Matériel Requis
#### Electronique :
- un controleur tel que l'arduino leonardo ou micro et des cables de prototypage
- un module PCA9685
- 7 servomoteurs 9g bas de gamme (avec dent metalique) => 6 doigts et 1 servo air flow
- Alimentation 5V 5A minimum
- Un solenoide 5V/6V avec 2N min (ideal secutiré a 5N ) il faut viser 500mA maximum de consomation pour eviter la surchauffe (idéal autour de 300mA) 
- un mofset adapté a la puissance
- une diode de roue libre
- un condensateur adapté 
  
  #### Mécanique :
  
- un systeme de mousse a pores fermée environ 3mm d'epaisseur
- les doigts et supports imprimé en 3D
- servo valve imprimé en 3D avec 2 roulements 12x4x3 et 3vis/ecrous M3x20mm 
- une boite ou planche pour supportrer le tout 

## Premiere utilisation

#### Calibration 

avant de fixer les doigts, il faut que les servomoteur soit initialisé en position fermé a 90° 
Pour le servo AirFlow, il faut bien definir les 2 angles maximum de deplacement ( SERVO_VALVE_MIN_FLOW , SERVO_VALVE_MAX_FLOW ) en fonction du systeme utilisé



