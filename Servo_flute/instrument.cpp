#include "Instrument.h"

Instrument::Instrument() : servoController() {
  if (DEBUG) {
    Serial.println("DEBUG : Instrument--creation");
  } 
  //varibles for vibrato 
  isPlaying=false;
  vibratoActive=false;
  vibratoDirection=true;
  //init buton open finger
  pinMode(PIN_OPEN_FINGER, INPUT_PULLUP); 
  ButtonState = digitalRead(PIN_OPEN_FINGER);
 //test();
}
/*******************************************************************************
----------------              gestion notes on             --------------------
******************************************************************************/
void Instrument::noteOn(uint8_t midiNote, uint8_t velocity) {
  if ((midiNote>=FIRST_MIDI_NOTE)&&(midiNote<=(FIRST_MIDI_NOTE+NUMBER_NOTES))){     // si on peut jouer la note
    servoController.noteOn(midiNote-FIRST_MIDI_NOTE);                     // on met les doigts en position 
    servoController.SetAirFlow(velocity);     
    //******************************************** delay a supprimer ou reduire au min---------------------------------------------------------<<<<<<<<<<<<<<<<<<<<<<<
    //delay(80);
    isPlaying=true;
  }else{
    if (DEBUG) {Serial.println("DEBUG : instrument, Wrong MIDI noteOn");}
  } 
}

/*******************************************************************************
----------------              gestion notes off            --------------------
******************************************************************************/
void Instrument::noteOff(uint8_t midiNote) {
  if ((midiNote>=FIRST_MIDI_NOTE)&&(midiNote<=(FIRST_MIDI_NOTE+NUMBER_NOTES))){   // si on peut jouer la note
    isPlaying=false; 
    servoController.SetAirFlow(0);                                 
  }else{
    if (DEBUG) {Serial.println("DEBUG : instrument, Wrong MIDI noteOff");}
  } 
}

/*******************************************************************************
----------------       test pour experimentations          --------------------
******************************************************************************/
void Instrument:: test(){
  int i;
  for(i=FIRST_MIDI_NOTE;i<30+FIRST_MIDI_NOTE;i++){
    noteOn(i,50);//vient faire toutes les notes jouable de la flute
    delay(500); // 1/2 seconde noteOn
    noteOff(i);//essayer sans ?
    delay(500); // 1/2 seconde entre chaque note 

  }
}

/*******************************************************************************
----------------        Gestion du vibrato          --------------------
******************************************************************************/
// l'ouverture/fermeture de la valve de vibrato est geré par la fonction update() si vibratoActive=true
void Instrument:: modulationWheel(int value){
  if (value == 0){// si la valeur est a zero, on desactive le vibrato
    vibratoActive=false;
  }else {    //si le vibrato est actif, on change le bool et on indique le temps de cycle entre On/Off en fonction des valeurs recupéré dans settings
    vibratoActive=true;
    vibratoTime=((value - 1) * (VIBRATO_MAX - VIBRATO_MIN)) / 126 + VIBRATO_MIN;//definition du temps entre 2 varations d'angle servoVlave
  }
}

/*******************************************************************************
----------------                    UPDATE                --------------------
******************************************************************************/
void Instrument:: update(){
  servoController.update();

   /* ca me fait des bugs ...
   // Lecture de l'état du bouton
    bool etatEntree = digitalRead(PIN_OPEN_FINGER);
    if (etatEntree != ButtonState) {//si changement d'etat
      if (etatEntree == LOW) {
        Serial.println("DEBUG : instrument, bouton appuyé");
        servoController.openFingers(true);

      } else {
        Serial.println("DEBUG : instrument, bouton relaché");
        servoController.openFingers(false);
      }
      ButtonState=etatEntree;
    }*/

  if(vibratoActive){ // si le vibrato est actif
    if(isPlaying){// si on est en train de jouer une note
      // on vient faire bouegr servoVale en fct du rythme recu et des valeurs dans settings
      unsigned long currentTime = millis();
      if(currentTime>vibratoNextTime){
        if(vibratoDirection){
          servoController.SetAirFlow(servoValveAngle+VIBRATO_ANGLE);
          vibratoDirection=false;
          vibratoNextTime=currentTime+vibratoTime;
        }else{
          servoController.SetAirFlow(servoValveAngle-VIBRATO_ANGLE);
          vibratoDirection=true;
          vibratoNextTime=currentTime+vibratoTime;
        }
      }
    }
  }
}
