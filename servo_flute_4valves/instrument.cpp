#include "Instrument.h"

Instrument::Instrument() : servoController() {
  if (DEBUG) {
    Serial.println("DEBUG : Instrument--creation");
  } 

  isPlaying=false;
  //init sorties etvavles
  pinMode(PIN_AIR_VALVE1, OUTPUT);
  pinMode(PIN_AIR_VALVE2, OUTPUT);
  pinMode(PIN_AIR_VALVE3, OUTPUT);
  pinMode(PIN_AIR_VALVE4, OUTPUT);
}

/*******************************************************************************
----------------                    UPDATE                --------------------
******************************************************************************/
void Instrument:: update(){
  servoController.update();
  if(isActive){
    if(vibratoActive){
      // on vient alterner l'ouverture et la fermeture de la valve pour le vibrato au rythme choisis en fct des valeurs dans settings
      unsigned long currentTime = millis();
      if(currentTime>vibratoNextTime){
        if(vibratoValveActive){
          digitalWrite(PIN_AIR_VALVE4, 0);
          vibratoValveActive=false;
          vibratoNextTime=currentTime+vibratoTime;
        }else{
          digitalWrite(PIN_AIR_VALVE4, 1);
          vibratoValveActive=true;
          vibratoNextTime=currentTime+vibratoTime;
        }
      }
    }
  }
}
/*******************************************************************************
----------------              gestion notes on             --------------------
******************************************************************************/
void Instrument::noteOn(uint8_t midiNote, uint8_t velocity) {
  if ((midiNote>=FIRST_MIDI_NOTE)&&(midiNote<=(FIRST_MIDI_NOTE+NUMBER_NOTES))){     // si on peut jouer la note
    servoController.noteOn(FIRST_MIDI_NOTE-midiNote);                     // on met les doigts en position 
    //******************************************** delay a supprimer ou reduire au min---------------------------------------------------------<<<<<<<<<<<<<<<<<<<<<<<
    delay(200);
    openValve(velocity);                                                  // ouvre les valves d'air en fonction de la velocité
    isActive=true;
  }
  if (DEBUG) {
    Serial.println("DEBUG : instrument, Wrong MIDI noteOn");
  } 
}

/*******************************************************************************
----------------              gestion notes off            --------------------
******************************************************************************/
void Instrument::noteOff(uint8_t midiNote) {
  if ((midiNote>=FIRST_MIDI_NOTE)&&(midiNote<=(FIRST_MIDI_NOTE+NUMBER_NOTES))){   // si on peut jouer la note
    closeValve();                                                            // ferme la valve d'air           
    isActive=false;                                  
  }
  if (DEBUG) {
    Serial.println("DEBUG : instrument, Wrong MIDI noteOff");
  } 
}

/*******************************************************************************************************************************
----------------              gestion air            --------------------
-----------------------------------------------------------------------------
a revoir pour adapter avec plusieurs sorties de plusieurs debit 
principe de base :
valve 1 : debit air min tout les trous ouvert
valve 2 : debit air min tout les trous fermé
valve 3 : environ la moité du debit de la vavle 1
valve 4 : debit air pour vibrato, doit etre très faible
******************************************************************************/

//************************************ ouverture des valves en fct de la vélocité =>>>>> a revoir ?? 
void Instrument:: openValve(int velocity){
  if (velocity<20){
    digitalWrite(PIN_AIR_VALVE1, 1);
  }else if (velocity<50){
    digitalWrite(PIN_AIR_VALVE2, 1);
  }else if (velocity<100){
    digitalWrite(PIN_AIR_VALVE1, 1);
    digitalWrite(PIN_AIR_VALVE3, 1);
  }else if (velocity<120){
    digitalWrite(PIN_AIR_VALVE2, 1);
    digitalWrite(PIN_AIR_VALVE3, 1);
  }else{
    digitalWrite(PIN_AIR_VALVE1, 1);
    digitalWrite(PIN_AIR_VALVE2, 1);
    digitalWrite(PIN_AIR_VALVE3, 1);
  }
}

//*********************** fermeture valves air 
// on ferme tout sans reflechir 
void Instrument:: closeValve(){
  digitalWrite(PIN_AIR_VALVE1, 0);
  digitalWrite(PIN_AIR_VALVE2, 0);
  digitalWrite(PIN_AIR_VALVE3, 0);
  digitalWrite(PIN_AIR_VALVE4, 0);
}

//*********************  Gestion du vibrato 
// l'ouverture/fermeture de la valve de vibrato est geré par la fonction update() si vibratoActive=true
void Instrument:: modulationWheel(int value){
  if (value == 0){// si la valeur est a zero, on desactive le vibrato
    vibratoActive=false;
  }else {    //si le vibrato est actif, on change le bool et on indique le temps de cycle entre On/Off en fonction des valeurs recupéré dans settings
    vibratoActive=true;
    vibratoTime=((value - 1) * (MAXVIBRATO - MINVIBRATO)) / 126 + MINVIBRATO;
  }
}