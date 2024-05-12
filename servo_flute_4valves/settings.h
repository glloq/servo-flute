/***********************************************************************************************
----------------------------         SETTINGS               ------------------------------------
fichiers pour la configuration du systeme 
************************************************************************************************/
#ifndef SETTINGS_H
#define SETTINGS_H
#include "stdint.h"

#define DEBUG 0

#define PIN_OPEN_FINGER 5//pin bouton ouverture des doigts 

/*******************************************************************************
-------------------------         MIDI SETTINGS        ------------------------
******************************************************************************/

#define FIRST_MIDI_NOTE 72 


/*******************************************************************************
---------------------------         AIR MANAGER        ------------------------
******************************************************************************/
//valve arrivé air
#define PIN_AIR_VALVE1 6 //pin valve
#define PIN_AIR_VALVE2 7 //pin valve
#define PIN_AIR_VALVE3 8 //pin valve
#define PIN_AIR_VALVE4 9 //pin valve

#define MINVIBRATO 400
#define MAXVIBRATO 1000
/*******************************************************************************
---------------------------   FINGERS MANAGER ------------------------
******************************************************************************/
#define TIMEUNPOWER 1000 //temps pour desactiver l'alim de la carte pwm 
#define PIN_SERVOS_OFF 10//pin oe carte pwm pour couper l'alim des servomoteurs et limiter le bruits => on a 0 off a 1

#define NUM_SERVOS_FINGER 10
#define NUMBER_NOTES 31 // nombres de note jouables donc 21 ou jusqu'a 32 si on prend en compte les trous demis ouvert
#define ANGLE_OPEN 30 
#define ANGLE_HALF_OPEN 10
#define NUM_SERVOS 10

// Tableau des angles pour chaque doigts/servo en position fermé
const uint16_t closedAngles[NUM_SERVOS_FINGER] = {65, 90, 90, 80, 90, 90, 90, 108, 90, 108};

//sens de rotation des servomoteur 1 : sens horaire, -1:sens anti horaire
const int sensRotation[NUM_SERVOS_FINGER] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

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
