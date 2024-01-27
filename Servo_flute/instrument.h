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
  void openValve(bool state);
  void setFan(int speed);

public:
	Instrument();
	void noteOn (uint8_t midiNote, uint8_t velocity);
	void noteOff(uint8_t midiNote);
  void update();
};

#endif // INSTRUMENT_H
