#ifndef SERVOCONTROLLER_H
#define SERVOCONTROLLER_H
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "settings.h"

class ServoController {
private:
  Adafruit_PWMServoDriver pwm;
  void setServoAngle(uint8_t servoNum, uint16_t angle);
  void resetServosPosition();
  bool isPowered; 
  void desactivate(bool active); // desactiver tout les servo => utilisé après un certain temps sans notes
  unsigned long TimeLastAction;
  void checkPowerOn();
  //int FingerPosition[10]; // tableau position actuelle des doigts 
public:
  ServoController(); //initialise toutles servomoteurs 
  void noteOn(int numNote); //ouvre ou ferme les servo en fonction de la note voulue  
  void openFingers(bool open);//demande l'ouverture /fermeture des doigts avec le bouton
  void SetAirFlow (int velocity);//gere l'angle de servoValve
  void update();//desactive la carte pwm x ms après la derniere action
};

#endif // SERVOCONTROLLER_H

