/***********************************************************************************************
----------------------------         SETTINGS               ------------------------------------
fichiers pour la configuration du systeme 
************************************************************************************************/
#ifndef SETTINGS_H
#define SETTINGS_H
#include "stdint.h"
#define DEBUG 0

#define  PIN_OPEN_FINGER 10 //pin bouton ouverture des doigts 

/*******************************************************************************
-------------------------         MIDI SETTINGS        ------------------------
******************************************************************************/
#define FIRST_MIDI_NOTE 72

/*******************************************************************************
---------------------------         AIR MANAGER        ------------------------
******************************************************************************/
//reglage valve arrivé air
#define NUM_SERVO_VALVE 15 // position brachement servo valve sur le PCA9685
#define SERVO_VALVE_CLOSE 30 // angle pour bloquer l'air
#define SERVO_VALVE_MIN_FLOW 35 // Angle ouverture minimum de la valve
#define SERVO_VALVE_MAX_FLOW 45 // Angle ouverture max de la valve

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
#define NUMBER_NOTES 21 // nombres de note jouables donc 21 ou jusqu'a 32 si on prend en compte les trous demis ouvert
#define ANGLE_OPEN 35 // angle ouverture du trouo de la flute
#define ANGLE_HALF_OPEN 10 // test ouverture a moitié du trou => bien gerer avec la mousse ? 

// Tableau des angles pour chaque doigts/servo en position fermé (idealement entre 50 et 120 °)
const uint16_t closedAngles[NUMBER_SERVOS_FINGER] = {55, 100, 95, 85, 95, 100, 95, 108, 95, 108};

//sens de rotation des servomoteur 1 : sens horaire, -1:sens anti horaire
const int sensRotation[NUMBER_SERVOS_FINGER] = {1,1,1,1,1,1,1,1,1,1};

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
const uint16_t SERVO_PULSE_MIN = 500;
const uint16_t SERVO_PULSE_MAX = 2500;
const uint16_t SERVO_FREQUENCY = 50;

#endif
