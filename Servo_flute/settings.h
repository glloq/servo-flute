/***********************************************************************************************
----------------------------         SETTINGS               ------------------------------------
fichiers pour la configuration du systeme 
************************************************************************************************/
#ifndef SETTINGS_H
#define SETTINGS_H
#include "stdint.h"

#define DEBUG 0
/*******************************************************************************
-------------------------         MIDI SETTINGS        ------------------------
******************************************************************************/



/*******************************************************************************
---------------------------         AIR MANAGER        ------------------------
******************************************************************************/
//ventilateur 
#define PIN_PWM 6 // pin pour le pwm du ventilateur 
#define MIN_PWM 100// vitesse minimum du ventilateur pour qu'il tourne sans faire de bruits


//valve arrivé air
#define PIN_AIR_VALVE 5 //pin valve pour les notes ON

//servo debit air 
#define PIN_SERVO_AIR_FLOW 10 // pin sur le pca du servo pour le controle du debit d'air 
#define OPEN_AIR_FLOW_ANGLE 45
#define MIN_AIR_FLOW_ANGLE 70
#define MAX_AIR_FLOW_ANGLE 135
#define PIN_SERVO_OFF 7  // pin pour desactiver les servos (reduirt le bruit a l'arret )
#define TIME_BETWEEN_UPDATE 25 // attend 25ms entre chaque degres pour ouvrir de 45° en environ 1seconde


#define FAN_OFF_ACTIVE 1 // 0 ou 1 pour desactivé le ventilateur après un certain temps sans notes
#define TIME_FAN_OFF 10000 // attend 10 secondes sans notes avant de couper le ventilateur 
/*******************************************************************************
---------------------------   FINGERS MANAGER ------------------------
******************************************************************************/
#define NUM_SERVOS_FINGER 10
#define FIRST_MIDI_NOTE 72 
#define NUMBER_NOTES 21 // nombres de note jouables donc 21 ou jusqu'a 32 si on prend en compte les trous demis ouvert
#define ANGLE_OPEN 30
#define ANGLE_HALF_OPEN 10
#define NUM_SERVOS 10

// Tableau des angles pour chaque doigts/servo en position fermé
const uint16_t closedAngles[NUM_SERVOS_FINGER] = {90, 90, 90, 90, 90, 90, 90, 90, 90, 90};

//sens de rotation des servomoteur 1 : sens horaire, -1:sens anti horaire
const uint16_t sensRotation[NUM_SERVOS_FINGER] = {1,1,1,1,1,1,1,1,1,1};

/******************************************************************************
---tableau positions des doigts pour chaque note----
il y a 21 accords avec les simples position ouvertes/fermé
et 11 des plus avec les trous a moitié ouvert donc jusqu'a 32 notes 
---- 0= hole close------ 1= hole open ----   2= half open  ---
*/
const int finger_position[][10] = {
  {0,0,0,0,0,0,0,0,0,0},// Do5 => midi note N°72
  {0,0,0,0,0,0,0,0,0,1}, // midi note N°73
  {0,0,0,0,0,0,0,0,1,1}, // midi note N°74
  {0,0,0,0,0,0,0,1,1,1}, // etc
  {0,0,0,0,0,0,1,1,1,1},
  {0,0,0,0,0,1,1,1,1,1},
  {0,0,0,0,0,1,0,0,0,0},
  {0,0,0,0,1,0,0,0,0,0},
  {0,0,0,0,1,0,0,0,1,1},
  {0,0,0,0,1,1,1,1,1,1},
  {0,0,0,1,0,0,0,1,1,1},
  {0,0,0,1,1,1,1,1,1,1},
  {0,0,1,0,0,1,1,1,1,1},
  {0,1,0,0,0,1,1,1,1,1},
  {0,0,1,1,1,1,1,1,1,1},
  {0,1,0,0,1,1,1,1,1,1},
  {0,1,0,1,1,1,1,1,1,1},
  {1,0,0,1,1,1,1,1,1,1},
  {0,1,1,1,1,1,1,1,1,1},
  {1,1,0,1,1,1,1,1,1,1},
  {1,1,0,0,0,0,0,0,1,1},

  {2,0,0,0,0,1,1,1,1,1},
  {2,0,0,0,0,1,0,0,1,1},
  {2,0,0,0,1,0,1,1,0,0},
  {2,0,0,0,1,0,0,1,1,1},
  {2,0,0,0,1,0,0,0,0,0},
  {2,0,0,1,0,1,1,1,1,1},
  {2,0,0,1,1,1,1,1,1,1},
  {2,0,0,1,0,0,0,0,0,0},
  {2,0,0,1,0,0,1,1,1,1},
  {2,0,1,1,0,0,1,1,1,1},
  {2,0,2,0,0,1,0,0,0,0}
  };


/*******************************************************************************
-----------------------    parametres des servomoteurs ------------------------
******************************************************************************/
//reglages du PCA9685 pour des servo mf90 
#define PCA_ADRESS 

#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180
const uint16_t SERVO_PULSE_MIN = 500;
const uint16_t SERVO_PULSE_MAX = 2500;
const uint16_t SERVO_FREQUENCY = 50;

#endif
