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
  bool isPlaying; 
  bool isActive; 
  unsigned long nextUpdate; //timestamp pour le mouvement du servo airFlow après une note
  int positionAirFlowServo ;// position actuelle du servo airflow 

public:
  ServoController(); //initialise toutles servomoteurs et le tableau
  void noteOn(int numNote, int velocity); //ouvre ou ferme les servo en fonction de la note voulue + on peut ajouter un deplacement de l'angle servo airflow pour ajouter une compression 
  void noteOff(); // utile pour savoir comment ouvrir le servo airflow => si pas de note en cour alors on ouvre au max 
  void setAirFlow(int velocity);//vien ouvir ou fermer l'angle du servo air flow pour ajouter plus ou moins de pression/debit
  void updateAirFlow();// permet de ré ouvrir le servo doucement jusqu'au seuil max si pas de note en cours et jusqu'au debit min si note en cours
  //void openFingers(bool open);//demande l'ouverture /fermeture des doigts 
};

#endif // SERVOCONTROLLER_H

