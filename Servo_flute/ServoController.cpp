#include "ServoController.h"
#include "settings.h"

//-----------------------------------------------------------------------------------
ServoController::ServoController() {
  pwm = Adafruit_PWMServoDriver(); 
  if(pwm.begin()==false){
    Serial.println("DEBUG :pca i2c not found");
  }
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQUENCY);
  resetServosPosition();
  positionAirFlowServo = OPEN_AIR_FLOW_ANGLE;
  isPlaying=false;
}

//-----------------------------------------------------------------------------------
void ServoController::setServoAngle(uint8_t servoNum, uint16_t angle) {
  // Adaptation de l'angle en plage de pulsations pour Adafruit_ServoDriver
  uint16_t pulsation = map(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
  int analog_value = int(float(pulsation) / 1000000 * SERVO_FREQUENCY * 4096);
  pwm.setPWM(servoNum, 0, analog_value);
}

//-----------------------------------------------------------------------------------
void ServoController::resetServosPosition() {
  // Utilisé au démarrage pour déplacer les doigts en position initiale fermée
    bool isEven;
  for (uint8_t i = 0; i < NUM_SERVOS; ++i) {
    setServoAngle(i, closedAngles[i]);  
	  delay(500); // délai pour laisser les servos se déplacer
    if (DEBUG) {
      Serial.print("DEBUG :reset init fermeture servo");
      Serial.println(i);
    } 
  }
  //ouvre le servo air flow  a la position initiale
  setServoAngle(PIN_SERVO_AIR_FLOW, OPEN_AIR_FLOW_ANGLE);
  if (DEBUG) {
    Serial.println("DEBUG : servoController--init servos done");
  } 
}

/*-----------------------------------------------------------------------------------
//fonction pour l'ajout futur possible d'un bouton pour ouvrir/fermer les doigts
void ServoController::openFingers(bool open) {
  if (open){
    for (uint8_t i = 0; i < NUM_SERVOS; ++i) {
      setServoAngle(i, closedAngles[i]+ANGLE_OPEN);  
      delay(100); // délai pour laisser les servos se déplacer
    }
  }else{
    for (uint8_t i = 0; i < NUM_SERVOS; ++i) {
      setServoAngle(i, closedAngles[i]);  
      delay(100); // délais pour laisser les servos se déplacer
    }
  }
}*/

/*******************************************************************************
----------------------------- Finger position  -------------------------------
********************************************************************************/
//gestion des doigts pour les accords => viens ouvrir ou fermer les trous en fonction de l'accord choisi
void ServoController::noteOn(int numNote, int velocity){ 
  isPlaying=true;
  //met a jour la position des  doigts en fonction de la note demandée
  for(int i=0; i<10;i++){
    if(finger_position[numNote][i]==0){// position fermée 
      setServoAngle(i, closedAngles[i]);
    }else if(finger_position[numNote][i]==1){//position ouverte 
      setServoAngle(i, closedAngles[i]+(ANGLE_OPEN*sensRotation[i]));
    }else if(finger_position[numNote][i]==2){// demi ouverte
      setServoAngle(i, closedAngles[i]+(ANGLE_HALF_OPEN*sensRotation[i]));
    }
  }
setAirFlow(velocity);
} 

/*******************************************************************************
----------------------------- gestion Air flow  -------------------------------
********************************************************************************/
void ServoController::setAirFlow(int velocity) {
  //met l'angle du servo afin de fermer l'evavuation d'air et d'augmenter la pression dans le systeme de distribution
  // setServoAngle(PIN_SERVO_AIR_FLOW, MAX_AIR_FLOW_ANGLE+45);  

  //on pârt du principe que update viendra ouvrir le servo air flow jusqu'a MIN_AIR_FLOW_ANGLE meme avec une note en cours
  int angle;
  angle= map(1,254,velocity,MIN_AIR_FLOW_ANGLE,MAX_AIR_FLOW_ANGLE ); // renvoi l'angle ideal pour le debit d'air en fonction de la velocité
  setServoAngle(PIN_SERVO_AIR_FLOW, angle);
  positionAirFlowServo = OPEN_AIR_FLOW_ANGLE;

  //sinon peut ajouter un mouvement du paneau air flow a chaque note afin d'ajouter une surpression et augmenter le debut de la note 
  // il faudrait ajouter un eplacement d'environ 20° en plus ( en dessous du max servoflow) quqi sera réouvert par update 

}
//-----------------------------------------------------------------------------------
void ServoController::noteOff(){
  isPlaying=false;
  // l'ouverture du circuit d'air sera géré par update 
}

//-----------------------------------------------------------------------------------
void ServoController::updateAirFlow() {
  //permet de réouvir le servo airflow petit a petit 
  //il faut ouvrir le servo petit a petit jusqu'a la position ouverte => faire en sorte de garder l'angle en dessous d'un certain seuil si note en cours
  unsigned long currentTime = millis();
  if (currentTime  >= nextUpdate) {
    if(isPlaying){// si note en cours alors on ouvre que jusqu'a flow min 
      if(positionAirFlowServo > MIN_AIR_FLOW_ANGLE){
        positionAirFlowServo=positionAirFlowServo-1;
        setServoAngle(PIN_SERVO_AIR_FLOW, positionAirFlowServo);
      }
    }else{ // si pas de note en cours alors on peut réouvrir jusqu'au max pour laisser passer l'air et reduire le bruit du ventilo
      if(positionAirFlowServo > OPEN_AIR_FLOW_ANGLE){
        positionAirFlowServo=positionAirFlowServo-1;
        setServoAngle(PIN_SERVO_AIR_FLOW, positionAirFlowServo);
      }
    }
    nextUpdate=currentTime+TIME_BETWEEN_UPDATE;// timestamp de la prochaine mise a jour de la position du servo airflow 
  }
}
