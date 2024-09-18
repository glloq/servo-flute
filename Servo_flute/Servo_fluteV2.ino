/***********************************************************************************************
----------------------------    MIDI servo flute 10 trous   ------------------------------------
************************************************************************************************
Systeme construit pour le controle d'une flute a bec avec des servomoteur de type sg90 et une carte pac9685

les systeme recoit les messages midi via le cable usb, midiHandler s'occupe de dechiffrer les messages midi
instrument s'occupe de verifier si il peut jouer les notes recues et demande ServoController de les jouer si c'est possible
il y a 10 servo moteurs pour les 10 trous de la flute
pour gerer le flux d'air, 1 servomoteur est utilisé pour adapter le debit d'air necessaire (en fct de la velocité)


tout les parametres doivent etre mis dan settings.h afin de simplifier les adaptations au materiel 
Un autre fichier .ino sera fournit afin d'initialiser les servo a 90° lors du montage et aussi pour trouver le reglage 
de la position centrale du servo pour chaque doigts et pour la servoValve
************************************************************************************************/
#include <MIDIUSB.h>
#include "Instrument.h"
#include "MidiHandler.h"
#include "Arduino.h"

Instrument* instrument= nullptr;
MidiHandler* midiHandler= nullptr;

void setup() {
  Serial.begin(115200);
  delay(1000);
  while (!Serial) {
    delay(10); // Attendre que la connexion série soit établie
  }
  Serial.println("init servo flute ");
  instrument= new Instrument();
  midiHandler = new MidiHandler(*instrument);
  Serial.println("fin init");
}

void loop() {
  midiHandler->readMidi();
  instrument-> update();
}
