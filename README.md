# servo flute

> [!WARNING]
> projet en cours de construction.

## presentation du projet

Le systeme est concu pour jouer de la flute a bec automatiquement en fonction des messages midi recu (par usb uniquement mais le code est facilement adaptable).
dans le cas d'un message midi NoteOn, si la note peut etre jouée le systeme va :
- deplacer les doigts pour faire l'accord voulu
- deplacer le servo airFlow vers la plute entre l'angle MIN_SERVO_AIR_FLOW et MAX_SERVO_AIR_FLOW en fonction de la note demandée 


Dans le cas d'un message midi NoteOff, si la note peut etre jouée le systeme :
- ferme deplace le servo airflow vers l'exterieur

Ajout possible : 
- gestion du vibrato/modulation wheel => software avec servoFlow
- gestion du volume (en gros augmenter/reduire le debit d'air)  => software avec servoFlow
- une boucle de rétroaction pour un meilleur controle des notes jouée => necessite un capteur piezo + ampli ?
  
## Schema principe

#### Servo Finger
l'objectif est d'avoir quelque chose de simple (sans soudures) qui utilise au mieux un systeme de bras de levier afin d'avoir un controle plus precis du mouvement de chaque doigts.
Si on par du principe que tout les doigts vont etre deplacé en meme temps, il faudra au moins 8A prevu juste pour les servomoteurs
![Schema de principe](https://github.com/glloq/servo-flute/blob/main/img/schemasv4.png?raw=true)

L'idée est d'utiliser des fils de fers de 1 a 1.5mm de diametre pour relier les servomoteurs a chaque doigts, ce design permet l'ajout de "ressorts" en pliant les fil de fers en Z => ca reduit les risque de casse et permet d'appuyer un peut plus sur le trou.  
Il faut garder environ 10cm de chaque coté de la flute pour permetre le placement des servo

nous utiliserons un gros ventilateur radial controlé en on/off via un mofset et un servomoteur viendra mettre en rotation le systeme de "bouche" devant l'entrée d'air de la flute et la sortie du ventilateur.

il faut aussi penser a la gestion du volume et du vibrato dans le futur=> prevoir un code facilement adaptable

## Fichiers 3D

Le systeme est concu pour fonctionner avec une flute a bec bas de gamme acheté sur amazon, la position des doigts peut ne pas etre adapté a d'autres flutes a bec sans faire de changement sur les doigts ou le support de rotation des doigts( on peut ce permettre 1 a 2 mm de decallage avec l'utilisation de la mousse) 
verifier que les dimensions sont adapté a la flute en votre possession avant d'imprimer quoi que ce soit
![Dimensions flute](https://github.com/glloq/servo-flute/blob/main/img/dimenssionFlute.png?raw=true)

les fichiers STL sont dans le dossier stl 


## materiel necessaire 

#### General

- un controleur tel que l'arduino leonardo ou micro et des cables de prototypage
- un module PCA9685
  - 11 servomoteurs 9g bas de gamme (avec dent metalique) => 10 doigts et 1 servo air flow
  - Alimentation 5V pour les servomoteurs => les 11 servomoteurs bougent en meme temps, prevoir une puissance adapté aux servomoteurs (environ 10A)
- du fil de fer diametre 1mm
- un systeme de mousse  (bande de 5mm de large min environ 3mm d'epaisseur)
- une tige en metal de 3mm (j'ai utilisé un cintre) 
- les doigts et supports imprimé en 3D
- des vis a bois 3x35/40mm (2 par support servoFinger)
- 2 vois bois 3x15mm (pour fixer le support flute)
- Une planche de bois pour tenir tout les composants ( environ 40x25cm avec une profondeur de 150cm )



## Premiere utilisation

pour les doigts, il faut que les servomoteur soit initialisé en position doigts fermé (initialiser les servo a 90° avant de fixer les bras) et il faut permettre un deplacement de +/-45° du servo.
Chacune des position des doigts sera a initialiser dans le fichier setting.h 

Pour le servo AirFlow, il faut bien definir les 2 debit ( SERVO_VALVE_MIN_FLOW , SERVO_VALVE_MAX_FLOW ) en fonction de l'instrument et du systeme d'air.

=> je travaille pour faire un code qui aidera a la calibration dse doigts et du servoFlow
