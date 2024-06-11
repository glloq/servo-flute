#include "ServoController.h"
#include "settings.h"
//-----------------------------------------------------------------------------------
ServoController::ServoController() {
  pwm = Adafruit_PWMServoDriver(); 
  if(pwm.begin()==false){
    Serial.println("DEBUG ServoController :pca i2c not found");
  }
  //on active la carte 
  pinMode(PIN_SERVOS_OFF, OUTPUT);
  digitalWrite(PIN_SERVOS_OFF, LOW);//on active la carte de ctrl des servos
  isPowered=true;


  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQUENCY);
  resetServosPosition();// on initialise tout les servomoteur 
}

/*******************************************************************************
----------------        Gestion angle finger         --------------------
******************************************************************************/
void ServoController::setServoAngle(uint8_t servoNum, uint16_t angle) {
  TimeLastAction = millis();//on stocke le temps de la derniere action 
  checkPowerOn();
  // Adaptation de l'angle en plage de pulsations pour Adafruit_ServoDriver
  uint16_t pulsation = map(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE, SERVO_PULSE_MIN, SERVO_PULSE_MAX);
  int analog_value = int(float(pulsation) / 1000000 * SERVO_FREQUENCY * 4096);
  pwm.setPWM(servoNum, 0, analog_value);

}
/*******************************************************************************
----------------        Gestion angle servoValve         --------------------
******************************************************************************/
void ServoController::SetAirFlow (int angle) {
  setServoAngle(NUM_SERVO_VALVE, angle);
}

/*******************************************************************************
----------------        desactive l'alim des servo         --------------------
******************************************************************************/
void ServoController::desactivate(bool active){
  if (active){
  digitalWrite(PIN_SERVOS_OFF, LOW);//on active la carte de ctrl des servos
  }else{
  digitalWrite(PIN_SERVOS_OFF, HIGH);//on desactive la carte de ctrl des servos pour reduire le bruit des moteurs
  }
}

/*******************************************************************************
----------------      remet l'alim des servo si besoin          --------------------
******************************************************************************/
void ServoController::checkPowerOn(){
  
  if (isPowered==false){  //on verifie si la carte est active avant de bouger les doigts
  desactivate(true); // on active l'alim des servos de la carte mcp
  isPowered=true; // on met a jours l'etat d'alim de la carte mcp
  }
}

/*******************************************************************************
----------------        reset Servos Position       --------------------
******************************************************************************/
void ServoController::resetServosPosition() {
  // Utilisé au démarrage pour déplacer les doigts en position initiale fermée et le servoValve
  setServoAngle(NUM_SERVO_VALVE, SERVO_VALVE_CLOSE);
  //vient ouvrir tous les doigts
  for (uint8_t i = 0; i < NUMBER_SERVOS_FINGER; ++i) {
    setServoAngle(i, closedAngles[i]-(ANGLE_OPEN*sensRotation[i]));  // ouvert 
	  //delay(30); // délai pour laisser les servos se déplacer
    if (DEBUG) {
      Serial.print("DEBUG ServoController :reset init ouverture servo");
      Serial.println(i);
    } 
  }
  	  delay(3000); // délai pour laisser les servos se déplacer
     //ferme tout les trous 
  for (uint8_t i = 0; i < NUMBER_SERVOS_FINGER; ++i) {
    setServoAngle(i, closedAngles[i]); 
	  //delay(200); // délai pour laisser les servos se déplacer
    if (DEBUG) {
      Serial.print("DEBUG ServoController :reset init fermeture servo");
      Serial.println(i);
    } 
  }
}

/*******************************************************************************
----------  demande ouverture des doigts avec bouton       --------------------
******************************************************************************/
void ServoController::openFingers(bool open) {

  if (open){
    for (uint8_t i = 0; i < NUMBER_SERVOS_FINGER; ++i) {
      setServoAngle(i, closedAngles[i]-(ANGLE_OPEN*sensRotation[i]));  
      delay(10); // délai pour laisser les servos se déplacer
    }
  }else{
    for (uint8_t i = 0; i < NUMBER_SERVOS_FINGER; ++i) {
      setServoAngle(i, closedAngles[i]);  
      delay(10); // délais pour laisser les servos se déplacer
    }
  }
}

/*******************************************************************************
----------------------------- Finger position  -------------------------------
********************************************************************************/
//gestion des doigts pour les accords => viens ouvrir ou fermer les trous en fonction de l'accord choisi
void ServoController::noteOn(int numNote){ 
   if (DEBUG) {
      Serial.print("DEBUG :servoControler noteOn :");
      Serial.println(numNote);
    } 
  //met a jour la position des  doigts en fonction de la note demandée
  for(int i=0; i<10;i++){
    if(finger_position[numNote][i]==0){// position fermée 
      setServoAngle(i, closedAngles[i]);
    }else if(finger_position[numNote][i]==1){//position ouverte 
      setServoAngle(i, closedAngles[i]-(ANGLE_OPEN*sensRotation[i]));
    }else if(finger_position[numNote][i]==2){// demi ouverte
      setServoAngle(i, closedAngles[i]-(ANGLE_HALF_OPEN*sensRotation[i]));
    }
  }
} 

/*******************************************************************************
----------------------------- Update -------------------------------
********************************************************************************/
void ServoController::update(){
  //gestion coupure alimentation des servomoteurs pour limiter le bruit 
  if (isPowered){
    unsigned long currentTime = millis();
    if(currentTime>(TimeLastAction+TIMEUNPOWER)){// si on depase le temps d'attente sans actions 
        desactivate(false); // on desactive l'alim des servos de la carte mcp
        isPowered=false; // on stocke l'etat d'alim de la carte mcp
    }
  }
}

