# servo flute
<! projet en cours de tests >

problemes actuel : 
  - le systeme de doigts n'est pas optimal et ne bouche pas bien le trou a cause de la mousse qui bouge=> il faudrait prevoir un embout en silicone ?
  - la gestion du deplacement de doigts lors de la reception d'une notOn bug certaines fois (avec des notes courtes ?) => probleme intensité trop faible ?? 
  - ajout d'une electrovanne 

## presentation du projet

Le systeme est concu pour jouer de la flute a bec automatiquement en fonction des messages midi recu (par usb uniquement).
dans le cas d'un message midi NoteOn, si la note peut etre jouée le systeme va :
- deplacer les doigts pour faire l'accord voulu
- deplacer le servo airFlow entre l'angle MIN_SERVO_AIR_FLOW et MAX_SERVO_AIR_FLOW en fonction de la note demandée
- ouvrir la valve d'air 

Dans le cas d'un message midi NoteOff, si la note peut etre jouée le systeme :
- ferme la valve d'air
  
Ajout possible : 
- gestion du vibrato/modulation wheel
- gestion du volume (en gros augmenter/reduire le debit d'air)
- une boucle de rétroaction pour un meilleur controle des notes jouée => necessite un capteur piezo + ampli ?
- un systeme de pompe et reserve d'air independant adapté => moteur + driver + capteur pression 
  
## Schema principe

Manque la valve general 
![Schema des doigts](https://github.com/glloq/servo-flute/blob/main/img/schemasDePrincipeV3.png?raw=true)

ajouter partie gestion air (moteur + systeme de pompe + 

## materiel necessaire 

- un controleur tel que l'arduino leonardo ou micro
- un module PCA9685
- 11 servomoteurs => 10 doigts et 1 servo air flow
- Alimentation 5V pour les servomoteurs => les 11 servomoteurs bougent en meme temps, prevoir une puissance adapté aux servomoteurs
  
- du fil de fer diametre 1mm
- un systeme de mousse isolante pour fenetre a coller (bande de 5mm de large min environ 3mm d'epaisseur)
- 10 roulements 4x13x5mm ?
- 1 vis M4x20 une tige filetée M4 coupé a la bonne longeur
- Une planche de bois pour tenir tout les composants

- toutes les pieces imprimé en 3D



## Fichiers 3D
Le systeme est concu pour fonctionner avec une flute a bec bas de gamme acheté sur amazon, la position des doigts peut ne pas etre adapté a d'autres flutes a bec sans faire de changement sur les doigts ou le support de rotation des doigts( on peut ce permettre 1 a 2 mm de decallage avec l'utilisation de la mousse) 
verifier que les dimensions sont adapté a la flute en votre pocession avant d'imprimer quoi que ce soit
![Dimensions flute](https://github.com/glloq/servo-flute/blob/main/img/dimenssionFlute.png?raw=true)

les fichiers STL sont dans le dossier stl 

## Premiere utilisation


pour les doigts, il faut que les servomoteur soit proche de 90° en position doigts fermé 
Pour la servoValve, il faut bien definir les 3 debit ( SERVO_VALVE_CLOSE ,SERVO_VALVE_MIN_FLOW , SERVO_VALVE_MAX_FLOW ) en fonction de l'instrument. 
les valeurs SERVO_VALVE_MIN_FLOW et SERVO_VALVE_MAX_FLOW sont très proche l'une de l'autre il faut definir SERVO_VALVE_MAX_FLOW avec tout les trou bouché et SERVO_VALVE_MIN_FLOW tout les trous ouvert

