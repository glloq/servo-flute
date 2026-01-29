# servo flute

## presentation du projet

Le systeme est concu pour jouer de la flute en fonction des messages midi recu 

dans le cas d'un message midi NoteOn, si la note peut etre jouée le systeme va :
- deplacer les doigts pour faire l'accord voulu
- deplacer le servo airFlow vers la plute entre l'angle MIN_SERVO_AIR_FLOW et MAX_SERVO_AIR_FLOW en fonction de la note demandée et de la velocité 
- ouvrir la vanne air vers la flute après un delais pour laisser le temps au servomoteur de se mettre en position

Dans le cas d'un message midi NoteOff, si la note peut etre jouée le systeme :
- fermer la vanne air vers la flute

  options supplementaire :
- CC1 : gestion du vibrato/modulation wheel => software avec servoFlow
- CC7/CC11 : gestion du volume (en gros augmenter/reduire le debit d'air)  => software avec servoFlow
- CC120 : all note Off / arret urgence

  
## Schema principe

#### Servo Finger
l'objectif est d'avoir quelque chose de simple (sans soudures), adaptable a plusieurs type d'instrument a vent similaire ( avec gestion air + 15 doigts max )
On utilisera une carte PCA9685 pour le controle des servomoteurs.
On utilisera un mofset avec un diode de roue libre pour la valve d'air 
Il faudra prevoir l'ajout d'un condessateur de decouplage adapté sur l'alimentation 5v pour limiter les chute de tension (470/1000nF) 


## materiel necessaire 
#### Electronique :
- un controleur tel que l'arduino leonardo ou micro et des cables de prototypage
- un module PCA9685
- 7 servomoteurs 9g bas de gamme (avec dent metalique) => 6 doigts et 1 servo air flow
- Alimentation 5V 5A minimum
- Un solenoide 5V/6V avec 2N min (ideal secutiré a 5N ) il faut viser 500mA maximum de consomation pour eviter la surchauffe (idéal autour de 30m0A) 
- un mofset adapté a la puissance
- une diode de roue libre
  
  #### Mécanique :
- du fil de fer diametre 1mm (pour lier les servomoteurs aux doigts) 
- un systeme de mousse a pores fermée environ 3mm d'epaisseur
- les doigts et supports imprimé en 3D
- servo valve imprimé en 3D avec 2 roulements 12x4x3 et 3vis/ecrous M3x20mm 
- une boite ou planche pour supportrer le tout 



## Premiere utilisation

pour les doigts, il faut que les servomoteur soit initialisé en position doigts fermé (initialiser les servo a 90° avant de fixer les bras) et il faut permettre un deplacement de +/-45° du servo.
Chacune des position des doigts sera a initialiser dans le fichier setting.h 

Pour le servo AirFlow, il faut bien definir les 2 debit ( SERVO_VALVE_MIN_FLOW , SERVO_VALVE_MAX_FLOW ) en fonction de l'instrument et du systeme d'air.

=> je travaille pour faire un code qui aidera a la calibration dse doigts et du servoFlow
