/***********************************************************************************************
----------------------------         SETTINGS               ------------------------------------
fichiers pour la configuration du systeme 
************************************************************************************************/
#ifndef SETTINGS_H
#define SETTINGS_H
#include "stdint.h"
#define DEBUG 1

#define  PIN_OPEN_FINGER 10 //pin bouton ouverture des doigts 

/*******************************************************************************
-------------------------         MIDI SETTINGS        ------------------------
******************************************************************************/
#define FIRST_MIDI_NOTE 72

/*******************************************************************************
---------------------------         AIR MANAGER        ------------------------
******************************************************************************/

//reglage valve servo flow
#define NUM_SERVO_VALVE 15 // position brachement servo flow sur le PCA9685
#define SERVO_VALVE_FLOW_OFF 20 // Angle servo pour note off
#define SERVO_VALVE_MIN_FLOW 60// Angle servo note faible
#define SERVO_VALVE_MAX_FLOW 100 // Angle servo note forte 



//reglage vibrato/modulation 
#define VIBRATO_MAX 100 // temps minimum entre 2 variations d'angle de servo valve
#define VIBRATO_MIN 1000 // temps max entre 2 variations d'angle de servo valve
#define VIBRATO_ANGLE 5 // angle de variation de servo valve en degres pour un "vibrato"

/*******************************************************************************
---------------------------   FINGERS MANAGER ------------------------
******************************************************************************/
#define TIMEUNPOWER 200 //temps pour desactiver l'alim de la carte pwm 
#define PIN_SERVOS_OFF 5//pin oe carte pwm pour couper l'alim des servomoteurs et limiter le bruits => on a 0 off a 1

#define NUMBER_SERVOS_FINGER 10 // nombre de servo pour les doigts 
#define NUMBER_NOTES 30 // nombres de note jouables donc 20 ou jusqu'a 32 si on prend en compte les trous demis ouvert
#define ANGLE_OPEN 30 // angle ouverture du trou de la flute
#define ANGLE_HALF_OPEN 10 // test ouverture a moitié du trou => bien gerer avec la mousse ? 

// Tableau des angles pour chaque doigts/servo en position fermé (idealement entre 70 et 110 °)
const uint16_t closedAngles[NUMBER_SERVOS_FINGER] = {90, 100, 95, 100, 90, 95, 90, 90, 100, 90};

//sens de rotation des servomoteur 1 : sens horaire, -1:sens anti horaire
const int sensRotation[NUMBER_SERVOS_FINGER] = {-1,1,1,1,1,-1,-1,1,1,1};

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

#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180
const uint16_t SERVO_PULSE_MIN = 550;
const uint16_t SERVO_PULSE_MAX = 2450;
const uint16_t SERVO_FREQUENCY = 50;

#endif
