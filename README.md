# servo flute

Schema des doigts

Schema du systeme d'air 

## materiel necessaire 

- un controleur tel que l'arduino leonardo
- un module PCA9685
- 11 servomoteurs => 10 doigts et 1 servo air flow
- du fil de fer diametre 1mm
- un systeme de mousse isolante pour fenetre a coller (bande de 5mm de large min environ 3mm d'epaisseur)
- 10 roulements 4x13x5mm
- Alimentation 5V pour les servomoteurs => les 11 servomoteurs bougent en meme temps, prevoir une puissance adapté aux servomoteurs
  
- 2 mofsets  
- Un ventilateur 12V suffisament gros pour donner un debit et pression d'air necessaire pour produire le son
- Une alimentation 12V
- Un systeme de valve
  - un solenoide 12v
  - un joint torique de 15x1mm
  - un bout de tube pcb pour cable electrique diametre 17mm
 
- Un systeme de support en bois pour tenir tout les composants
- toutes les pieces imprimé en 3D => viendrons quand tout fonctionne mais je sis pas certain que ce s'adapte facilement aux autres flutes 

## Fichiers 3D

-- a venir --
ajouter un plan pour la position des trous ??

### premiere utilisation 
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

