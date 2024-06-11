#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include "settings.h"
#include "ServoController.h"
/***********************************************************************************************
----------------------------    instrument.h   ----------------------------------------
************************************************************************************************
execute les messages noteOn et noteOff
gere la vavle d'air et le vantilateur, le controle des servo passe par servoControler
update permet de mettre a jours l'angle du servo airflow
************************************************************************************************/
class Instrument {
private:
  ServoController servoController;
  void openValve(int velocity);
  void closeValve();
  bool isActive;//boolean pour couper l'alim des servos via update
  bool isPlaying; //stocke si en train de jouer une note
  
  //gestion angle ouverture valve
  int servoValveAngle;//on stocke l'angle actuel de servoValve 

  //gestion du vibrato 
  bool vibratoActive; // stocke si le vibrato est demandé ou non
  bool vibratoDirection; // stocke le sens pour chaque mouvement du servo valve 
  int vibratoTime;//temps entre 2 changement de debit d'air
  unsigned long vibratoNextTime; //temps prochain changement 
  
public:
	Instrument();
	void noteOn (uint8_t midiNote, uint8_t velocity);// message midi noteOn
	void noteOff(uint8_t midiNote); // message midi noteOn
  void modulationWheel(int value); // message midi vibrato
  void update();//gere le vibrato et l'extinction des servos après un certain temps 

  void test();
};

#endif // INSTRUMENT_H
