#include "Instrument.h"

Instrument::Instrument() : servoController() {
  if (DEBUG) {
    Serial.println("DEBUG : Instrument--creation");
  } 
  //init sorties et objets ventilo, vavle
  pinMode(PIN_AIR_VALVE, OUTPUT);
  pinMode(PIN_PWM, OUTPUT);
}

void Instrument::noteOn(uint8_t midiNote, uint8_t velocity) {
  if ((midiNote>=FIRST_MIDI_NOTE)&&(midiNote<=(FIRST_MIDI_NOTE+NUMBER_NOTES))){     // si on peut jouer la note
    servoController.noteOn(FIRST_MIDI_NOTE-midiNote,velocity);                     // on met les doigts en position 
    setFan(100);                                                                  // met le ventilateur a 100%
    openValve(true);                                                             // ouvre la valve d'air
  }
}

void Instrument::noteOff(uint8_t midiNote) {
  if ((midiNote>=FIRST_MIDI_NOTE)&&(midiNote<=(FIRST_MIDI_NOTE+NUMBER_NOTES))){   // si on peut jouer la note
    openValve(false);                                                            // ferme la valve d'air 
    setFan(80);                                                                 // reduit la vitesse du ventilateur                                              
  }
}

void Instrument::openValve(bool state){
  digitalWrite(PIN_AIR_VALVE, state);
}

void Instrument::setFan(int speed){
  analogWrite(PIN_PWM, constrain(speed, MIN_PWM, 255));                       // on varie le pwm entre min et max 
}

//met a jour tout les objets 
void Instrument::update(){ 
  servoController.updateAirFlow();                                            //augmente l'angle du servo airflow 
  //ajouter une gestion du ventilateur ??
}