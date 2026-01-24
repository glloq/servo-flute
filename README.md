# servo flute

> [!WARNING]
> projet en cours de construction.

## presentation du projet

Le systeme est concu pour jouer de la flute a bec automatiquement en fonction des messages midi recu (par usb uniquement mais le code est facilement adaptable).
dans le cas d'un message midi NoteOn, si la note peut etre jouée le systeme va :
- deplacer les doigts pour faire l'accord voulu
- deplacer le servo airFlow vers la plute entre l'angle MIN_SERVO_AIR_FLOW et MAX_SERVO_AIR_FLOW en fonction de la note demandée et de la velocité 
- ouvrir la vanne air vers la flute

Dans le cas d'un message midi NoteOff, si la note peut etre jouée le systeme :
- fermer la vanne air vers la flute
- gestion du vibrato/modulation wheel => software avec servoFlow
- gestion du volume (en gros augmenter/reduire le debit d'air)  => software avec servoFlow

Ajout possible : 
- une boucle de rétroaction pour un meilleur controle des notes jouée
  
## Schema principe

#### Servo Finger
l'objectif est d'avoir quelque chose de simple (sans soudures) qui utilise au mieux un systeme de bras de levier afin d'avoir un controle plus precis du mouvement de chaque doigts.
Si on par du principe que tout les doigts vont etre deplacé en meme temps, il faudra au moins 8A prevu juste pour les servomoteurs
> [!WARNING]
> je garde les doigts mais je change la partie pompe => schema a refaire avec partie pompe separée
![Schema de principe](https://github.com/glloq/servo-flute/blob/main/img/schemasv4.png?raw=true)

L'idée est d'utiliser des fils de fers de 1 a 1.5mm de diametre pour relier les servomoteurs a chaque doigts, ce design permet l'ajout de "ressorts" en pliant les fil de fers en Z => ca reduit les risque de casse et permet d'appuyer un peut plus sur le trou.  
Il faut garder environ 10cm de chaque coté de la flute pour permetre le placement des servo

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
