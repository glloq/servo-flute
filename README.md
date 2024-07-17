# servo flute
<! projet en cours de tests >

## presentation du projet

Le systeme est concu pour jouer de la flute a bec automatiquement en fonction des messages midi recu (par usb uniquement mais le code est facilement adaptable).
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

#### Servo Finger
l'objectif est d'avoir quelque chose de simple (sans soudures) qui utilise au mieux un systeme de bras de levier afin d'avoir un controle plus precis du mouvement de chaque doigts.
Si on par du principe que tout les doigts vont etre deplacé en meme temps, il faudra au moins 8A prevu juste pour les servomoteurs
![Schema des doigts](https://github.com/glloq/servo-flute/blob/main/img/schemasDePrincipeV3.png?raw=true)

#### Air Manager

l'objectif est d'avoir un systeme de pompes et reserves a souflet le plus compact possible.
on peut estimer la consomation d'air d'une flute autour de 2 a 3 litres/minutes lors d'un jeu modéré. Nous viserons une consomation max autour de 5 l/min.
il faut donc essayer d'avoir un systeme de pompe silencieux qui est capable de fournir 5 l/min avec assez de force pour remplir une reserve d'environ 2 litres.
il faut aussi respecter des contraintes de tailles pour eviter d'avoir un systeme trop gros.

On va commencer par des pompes carré de 11x11cm avec un deplacement de 4cm 

 ###### Les pompes
 


idealement 2 a 3 petites pompes permettrait un aport d'air correct, il nous faut un moteur silencieux qui tourne a une vitesse sufisante pour avoir notre cible de 5 l/min.


 ###### La reserve d'air

Le systeme de reserve devras etre maintenu sous pression avec des poids ou un ou pluseurs ressorts.
on a plusieurs facons de controler le volume d'air disponible dans le soufflet de reserve: 
- un potentiometre lineaire relié a la pompe a soufflet
- un systeme de capteur de pression
- un systeme de mesure de distance par ultrason




## Fichiers 3D
Le systeme est concu pour fonctionner avec une flute a bec bas de gamme acheté sur amazon, la position des doigts peut ne pas etre adapté a d'autres flutes a bec sans faire de changement sur les doigts ou le support de rotation des doigts( on peut ce permettre 1 a 2 mm de decallage avec l'utilisation de la mousse) 
verifier que les dimensions sont adapté a la flute en votre possession avant d'imprimer quoi que ce soit
![Dimensions flute](https://github.com/glloq/servo-flute/blob/main/img/dimenssionFlute.png?raw=true)

les fichiers STL sont dans le dossier stl 


## materiel necessaire 

- un controleur tel que l'arduino leonardo ou micro
- un module PCA9685
- 11 servomoteurs => 10 doigts et 1 servo air flow
- Alimentation 5V pour les servomoteurs => les 11 servomoteurs bougent en meme temps, prevoir une puissance adapté aux servomoteurs (environ 8A minimum)
- une electrovanne 12V (au moins 2mm de diametre de sortie ou un debit de 0.23cv) 
  
- du fil de fer diametre 1mm
- un systeme de mousse isolante pour fenetre a coller (bande de 5mm de large min environ 3mm d'epaisseur)
- une tige en metal de 3mm (j'ai utilisé un cintre) 
- Une planche de bois pour tenir tout les composants



- toutes les pieces imprimé en 3D => a venir lorsque le projet est validé 

## Premiere utilisation

pour les doigts, il faut que les servomoteur soit initialisé en position doigts fermé (initialiser les servo avant de fixer les bras) et il faut permettre un deplacement de 90° du servo.
Pour le servo AirFlow, il faut bien definir les 2 debit ( SERVO_VALVE_MIN_FLOW , SERVO_VALVE_MAX_FLOW ) en fonction de l'instrument et du systeme d'air.


