# servo flute
<! projet en cours non testé >

## presentation du projet

Le systeme est concu pour jouer de la flute a bec automatiquement en fonction des messages midi recu (par usb uniquement).
dans le cas d'un message midi NoteOn, si la note peut etre jouée le systeme va :
- deplacer les doigts pour faire l'accord voulu
- deplacer le servo airFlow entre l'angle MIN_SERVO_AIR_FLOW et MAX_SERVO_AIR_FLOW en fonction de la velocité
- ouvrir la ou les valves d'air vers la flute en fonction de la velocité

Après une noteOn, le servo air flow s'ouvrira de 1 degres toute les 25ms ( TIME_BETWEEN_UPDATE ) jusqu'a la limite min (MIN_SERVO_AIR_FLOW) si la note est active et jusqu'au max ( OPEN_SERVO_AIR_FLOW ) a la meme vitesse 

dans le cas d'un message midi NoteOff, si la note peut etre jouée le systeme :
- ferme la ou les valves d'air vers la flute

## Schema principe
![Schema des doigts](https://github.com/glloq/servo-flute/blob/main/img/schemasCompletv2.png?raw=true)


## materiel necessaire 

- un controleur tel que l'arduino leonardo
- un module PCA9685
- 11 servomoteurs => 10 doigts et 1 servo air flow
- du fil de fer diametre 1mm
- un systeme de mousse isolante pour fenetre a coller (bande de 5mm de large min environ 3mm d'epaisseur)
- 10 roulements 4x13x5mm
- 1 vis M4x20 une tige filetée M4 coupé a la bonne longeur
- Alimentation 5V pour les servomoteurs => les 11 servomoteurs bougent en meme temps, prevoir une puissance adapté aux servomoteurs
  
- 2 mofsets  
- Une alimentation 12V
- Un systeme de valve
  - un solenoide 12v
  - une diode
  - un joint torique de 15x1mm
  - un bout de tube pcb pour cable electrique diametre externe 17mm
 
- Un systeme de support en bois pour tenir tout les composants

- toutes les pieces imprimé en 3D



## Fichiers 3D
Le systeme est concu pour fonctionner avec une flute a bec bas de gamme acheté sur amazon, la position des doigts peut ne pas etre adapté a d'autres flutes a bec sans faire de changement sur les doigts ou le support de rotation des doigts( on peut ce permettre 1 a 2 mm de decallage avec l'utilisation de la mousse) 
verifier que les dimensions sont adapté a la flute en votre pocession avant d'imprimer quoi que ce soit
![Dimensions flute](https://github.com/glloq/servo-flute/blob/main/img/dimenssionFlute.png?raw=true)

-- a venir --

## le support en bois


### premiere utilisation 

 => encore plein de bugs :S

j'ai crée un code pour trouver les parametres a copier coller dans settings.h
charger le fichier start.ino et televerser sur le microcontroleur
ouvrir la console et suivre les information pour choisir le reglage (doigts ou air flow)

parties doigts :

deplacer de +/-1,+/-5 ou +/-10 degrès chaque servo l'un après l'autre

avant de passer a la note suivante, pensez a tester le changement de note en activant le ventilateur et le solenoide avec la commande indiqué

une fois tout les servo initialisé, copier le resultat dans settings.h

partie air flow :

il faut definir la position air MIN_SERVO_AIR_FLOW avec tout les trous bouché car c'est la plus grande consomation qu'il y aura pour la flute

le servo air flow sera initilialisé a la position minimum (MIN_SERVO_AIR_FLOW), la valve ouverte et le ventilateur au maximum

changer l'angle de la meme maniere qu'avec les doigts jusqu'a avoir le bon son, ce sera la position MIN_SERVO_AIR_FLOW a mettre dans settings.h

