# servo flute
<! projet en cours >

## presentation du projet

Le systeme est concu pour jouer de la flute a bec automatiquement en fonction des messages midi recu (par usb uniquement mais le code est facilement adaptable).
dans le cas d'un message midi NoteOn, si la note peut etre jouée le systeme va :
- deplacer les doigts pour faire l'accord voulu
- deplacer le servo airFlow entre l'angle MIN_SERVO_AIR_FLOW et MAX_SERVO_AIR_FLOW en fonction de la note demandée 
- ouvrir la valve d'air 

Dans le cas d'un message midi NoteOff, si la note peut etre jouée le systeme :
- ferme la valve d'air

Ajout en cours : 
- pompes et reserves d'air

Ajout possible : 
- gestion du vibrato/modulation wheel => software avec servoFlow
- gestion du volume (en gros augmenter/reduire le debit d'air)  => software avec servoFlow
- une boucle de rétroaction pour un meilleur controle des notes jouée => necessite un capteur piezo + ampli ?
  
## Schema principe

#### Servo Finger
l'objectif est d'avoir quelque chose de simple (sans soudures) qui utilise au mieux un systeme de bras de levier afin d'avoir un controle plus precis du mouvement de chaque doigts.
Si on par du principe que tout les doigts vont etre deplacé en meme temps, il faudra au moins 8A prevu juste pour les servomoteurs
![Schema des doigts](https://github.com/glloq/servo-flute/blob/main/img/schemasfingers.png?raw=true)

L'idée est d'utiliser des fils de fers de 1 a 1.5mm de diametre pour relier les servomoteurs a chaque doigts, ce design permet l'ajout de "ressorts" en pliant les fil de fers en Z => ca reduit les risque de casse et permet d'appuyer un peut plus sur le trou.
Il faut garder environ 10cm de chaque coté de la flute pour permetre le placement des servo

#### Air Manager

l'objectif est d'avoir un systeme de pompes et reserves a souflet le plus compact possible.
on peut estimer la consomation d'air d'une flute autour de 2 a 3 litres/minutes lors d'un jeu modéré et une consomation max instantané autour de 0.4 l/s soit environ 24 l/min et une pression de reserve autour de 3 a 4 KPa.
![Schema des doigts](https://github.com/glloq/servo-flute/blob/main/img/schemaspompes.png?raw=true)

 ###### Les pompes
 
idealement 2  pompes permettrait un aport d'air correct, il nous faut un moteur silencieux qui tourne a une vitesse sufisante pour avoir notre cible de 0.4 l/sec.
Il y a beaucoup de facon de faire le systeme de pompes : pompe a verin, a soufflet, centrifuge etc ... 
Nous ferons un systeme de 2 pompes a soufflet inspiré par les systemes de soufflet d'orgues.
il faut idealement eviter d'utiliser des pompes trop grosses, nous viserons une largeur maximale de 10cm et un deplacement de 4 a 5 cm.

Le moteur devrais idealement etre arrété lorsque la pompe est pleine mais il est possible de devoir faire tourner le moteur a faible vitesse pour compensser les pertes par fuite => à adapter en fonction de la construction :/

Pour un soucis de maitenance et pour rendre le tout plus adaptable aux materiaux trouvé pour faire les valves, nous utiliserons un systeme de trappe d'acces sur laquelle on fixera les valves, une mousse pour eviter la poussiere à l'entrée d'air et une boite de redirection vers la reserve à la sortie d'air.

   ![Schema trappe valves](https://github.com/glloq/servo-flute/blob/main/img/trappe%20soufflet.png?raw=true)


on peut estimer le volume d'air envoyé par un cycle de la pompe avec le calcul suivant:
( volume soufflet x hauteur ouverture  / 2 ) x 0.7  => on retire 30% pour le volume retiré par les eclisses des pompes.

Voici un exemple de dimenssions qui pourrait etre adapté :
2 pompes de 10x25cm avec un deplacement de 5cm fournirait environ 26 litres par minutes avec un moteur qui tourne a 30tr/min 

 ###### La reserve d'air

Le systeme de reserve devras etre maintenu sous pression avec l'utilisation de ou un ou pluseurs ressorts (l'ajout de poids est ausi possible) 
on va utilise run capteur a effet hall pour connaitre precisement la position de la partie supperieur de la reserve => la vitesse de rotation du moteur actionnant les pompes sera faite en fonction de la valeur du capteur a effet hall ( on reste autour de la meme pression d'utilisation avec un volume de securité autour de 5 a 10 secondes => entre 1 a 2 litres)

 ###### Les vannes 
 
il y a 2 type de vannes utilisé dans ce projet : 

- la servo vanne :
  ![Schema servoFlow](https://github.com/glloq/servo-flute/blob/main/img/servo%20vavle%20variable.png?raw=true)
  - le diametre minimum du passage d'air (tuyaux et pieces imprimé) est a 6mm pour limiter les pertes.
  - Le debit doit etre adapté en fonction de la note jouée; une note grave consomera plus d'air qu'une note aigue.
  - le servomoteur "servoFlow" permet de controler le debit d'air en comprimant plus ou moins un tube en silicone qui alimente la flute en air.
  -Après plusieurs test, un tuyau en silicone de diametre exterieur 8 mm et de diametre interieur 6mm fonctionne parfaitement pour gerer le debit d'air avec precision.


- la vanne principale : une vanne faite avec un solenoide 5 ou 12v a la sortie du reservoir d'air
  ![Schema vanne air](https://github.com/glloq/servo-flute/blob/main/img/vanne%20generale.png?raw=true)
  - il faut absolument poncer au plus lisse possible la parroie ou il y a le trou d'evacuation d'air, une parroie lisse aidera beaucoup pour eviter les fuites d'airs.
  - utiliser de la colle chaude pour recourvir le passage des cables et recouvrir les tetes des boulons pour eviter toute fuites d'air
  - Pour eviter les problemes d'alignement, on utilisera des patins en mousse très souple entre lr support imprimé et la partie qui fait "joint" (plaque a joint, cuir ,caoutchouc, siline, etc ...)
  - il faudra aussi prevoir une plaque a joint que l'ont peut decouper pour limiter les fuites d'air de la valve au maximum (plaque de 10x4cm)
  - il faut prevoir un solenoide avec un deplacement de 10mm et un couple autour de 3 a 5N, ont peut choisir un solenoide moyen/bas de gamme car il restera ventilé en fonctionement.
  - le solenoide devra etre legerement mis en pression afin d'appuyer sur le joint de la valve (et le couple est plus important plus on se rapproche de la position alimenté/activé) 
  - La vis de reglage permet de limiter le trajet de la tige du solenoide, l'objectif est de limiter le trajet afin d'avoir une noteOff plus rapide.

 il faut penser que les notes grave necessitent plus de debit que les notes aigues pour rester au meme niveau sonore, il faut donc adapter l'ouverture du servoFlow en fonction de la note et ajouter un delta en fonction de la velocité demandé.
 
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

#### air manager

- un solenoide de 6V 3 à 5 N
  - un transitor ou mofset avec diode de roue libre
- un roulement de 4x12x5mm
- 2 boulon + ecrous M4x25mm
- 8 boulon + ecrous m3x20mm


## Premiere utilisation

pour les doigts, il faut que les servomoteur soit initialisé en position doigts fermé (initialiser les servo a 90° avant de fixer les bras) et il faut permettre un deplacement de +/-45° du servo.
Chacune des position des doigts sera a initialiser dans le fichier setting.h 

Pour le servo AirFlow, il faut bien definir les 2 debit ( SERVO_VALVE_MIN_FLOW , SERVO_VALVE_MAX_FLOW ) en fonction de l'instrument et du systeme d'air.

=> je travaille pour faire un code qui aidera a la calibration dse doigts et du servoFlow
