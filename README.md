# servo flute
<! projet en cours de tests >

## presentation du projet

Le systeme est concu pour jouer de la flute a bec automatiquement en fonction des messages midi recu (par usb uniquement).
dans le cas d'un message midi NoteOn, si la note peut etre jouée le systeme va :
- deplacer les doigts pour faire l'accord voulu
- deplacer le servo airFlow entre l'angle MIN_SERVO_AIR_FLOW et MAX_SERVO_AIR_FLOW en fonction de la velocité

dans le cas d'un message midi NoteOff, si la note peut etre jouée le systeme :
- ferme la valve d'air vers la flute

## Schema principe
![Schema des doigts](https://github.com/glloq/servo-flute/blob/main/img/schemasComplet.png?raw=true)

## materiel necessaire 

- un controleur tel que l'arduino leonardo ou micro
- un module PCA9685
- 11 servomoteurs => 10 doigts et 1 servo air flow
- Alimentation 5V pour les servomoteurs => les 11 servomoteurs bougent en meme temps, prevoir une puissance adapté aux servomoteurs
  
- du fil de fer diametre 1mm
- un systeme de mousse isolante pour fenetre a coller (bande de 5mm de large min environ 3mm d'epaisseur)
- 10 roulements 4x13x5mm ?
- 1 vis M4x20 une tige filetée M4 coupé a la bonne longeur
- Un systeme de support en bois pour tenir tout les composants

- toutes les pieces imprimé en 3D



## Fichiers 3D
Le systeme est concu pour fonctionner avec une flute a bec bas de gamme acheté sur amazon, la position des doigts peut ne pas etre adapté a d'autres flutes a bec sans faire de changement sur les doigts ou le support de rotation des doigts( on peut ce permettre 1 a 2 mm de decallage avec l'utilisation de la mousse) 
verifier que les dimensions sont adapté a la flute en votre pocession avant d'imprimer quoi que ce soit
![Dimensions flute](https://github.com/glloq/servo-flute/blob/main/img/dimenssionFlute.png?raw=true)

-- a venir --

## le support en bois

-- a venir --
 
## Premiere utilisation

