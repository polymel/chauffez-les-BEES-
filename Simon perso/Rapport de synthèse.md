Ce fichier servira à suivre l'avancement de ce que j'ai fait lors des séances d'électronique

il fait suite au premier rapport de synthèse du 23-09-24 se trouvant dans un pdf du même dossier.

12/11/2024:?

25/11/2024:j'ai commencé un code arduino pour afficher directement les données sur l'esp32 lora dans le fichier: [controleesp32](Simon perso\controleesp32)

09/12/2024: on n'utilise plus l'esp32 lora mais l'esp wroom 32E ayant une antenne directement intégrée et sans affichage dessus, le code qui a été développé ne sert donc plus.

next: 2 sorties pilotables CMOS sur le pcb

03/02/2025:

fin bom+schematique+ pcb + generation fichiers pour production

avec altium, pret à etre fabriquée

composants commandés

commencer rassemblement boulot des coéquipier pour l'esp 32.

Début de la préparation pour la présentation (lien vers présentation: [Oral S7 Chauffage ruche.pptx](https://unice-my.sharepoint.com/:p:/g/personal/melilla_caillot_etu_unice_fr/EexJpyUrVStCtURrQXf4pCQBGJ_DhOEJgqGgKRXciLH0BQ?e=O4ab3Y) )

le suivi est fait aussi grace au rapport sur l'historique du projet Altium (comme j'étais le seul à travailler dessus: [Log travail altium](Simon perso\log travail au 10_02_25 Simon sur le git.png))

lien pdf schematic:  [Altium Schematic&amp;BOM](Altium_PCB\Project Outputs for Altium_PCB\Schematic&BOM.PDF)

10/03/2025:

assemblage carte electronique avec pate à braser et four à fusion (sans C1, sera montée quand l'on branchera le système sur le panneau solaire)
matériel utilisé: Pate=>smdltlfp50T3; Four=> celui du Sofab;
reprise U3 ( pates interconnectées= pas bon)
reprise UT1 ( une des pates n'était pas connectée)
Ajout de UT4 ( avec étain )

11/03/2025:

verification connections avec un multimètre en mode test de continuité

remarque sur la carte après les tests:
-RPD2 texte est à l'envert
-le connecteur PT1 n'a pas de texte pour savoir comment le brancher
-R7,R8 sont à 4,7k car il n'y a pas de 3.3k (pour le pull up de l'i2c)
-R5,R6 sont à 10ohm pas à 10,2 (pas très grave, pour limiter le bruitage du signal)
-R3 est pas bon, il faut 470 ohm et pas 4,7kohm (voir montage de melilla)

mise sous tension et vérification de la tension de l'esp 32 ( à 3.3V) pour certaines tensions d'entrées données: 5,9,10,11,15,20V

si la tension est trop basses, le montage ne laissera pas passer le courant dans le ldo permettant l'alimentation de notre esp32.

tests à réaliser à la prochaine séance:

-verificaiton connection avec pc via le connecteur J2 et la carte d'adaptation uart to usbA,
-verification televersement programme sur esp 32 avec arduino IDE
-verification du bon fonctionnement des capteurs de température ( les 4 brasés sur la carte , il en manque un en externe)
-verification communication ina219 via l'i2c
-verification comme quoi la puissance est relevée ( tension et courant) sur l'ina219

24/03/25:

un problème de reset permanent a été découvert lors dutéléversage. il vient du hard avec le pin gpio 12 de l'esp32 étant lié à une fonction du microcontroleur. J'ai donc coupé laconnection sur le pcb et tiré un fil sur le pin gpio 13 pour relier data tempétature.

sur le code de Controle_all_esp32=>

-modif pin data temps : problème lors du reset sur IO12 donc passage a IO14.

-lecture température 4 capteurs sur la carte ( il manque le capteur externe qui arrivera plus tard)

25/03/25:

la communication entre l'ina219 et l'esp32 a marché mais il y avait un problème au niveau de la lecture du courrant. En effet, d'après le code de la lib choisit par Laura, il faut utiliser une seul et unique valeur de resistance de shunt pour notre système car la lecture du courrant est une image de la tension aux bornes de la shunt.

j'ai donc fait un  changement Rshunt ( 0.5 à 0.1 ohm) pour avoir la bonne valeur de la tension shunt

le code arduino a été amélioré pour une lecture plus claire des données (voir les images
avant:    [log arduino](Simon perso\Screenshot_Simon\Log esp32 wroom 24_03_25.txt)

et après: [log arduino 2](Simon perso\Screenshot_Simon\Log esp32 wroom 25_05_25.txt)

) Pour comprendre le type de message que l'on recoit, on met une lettre à chaque debut de ligne
I: Information
E: Erreur
M: Mesure

7/4/25:

Pour augmenter la tension Vconsigne , on fait une pwm sur le Cmos avec un rapport cyclique augmentant en sachant que 60% de rapport cyclique <=> 0.6*Vcourt circuit panneau sol

Methode de MPPT applicable: 3 facons différentes

FOCV: % de open circuit volt => marche mais que à 70/80% de rendement

PO: perturb & oberverve => plus précis car mesure la puissance en permanence et avec un variation de la tension sur la commande, on regarde si la puissance augmente ou diminue et en fonction de ce que l'on observe , on rechage la tension

exemple: si on a vàtester=Vconsigne+0.1 et que l'on voit que P(I sol*Vsol)augmente alors Vconsigne+=0.1 sinon on fait l'inverse  !!! au local maximal dans cette méthode

IC:  incremental Conductance: comme PO mais utilise directement la dérivée de la puissance par rapport à la tension pour déterminer avec précision la position du MPP.

si dP/dV>0 => V a augmenter

si dP/dV<0 => V a diminuer

si dP/dV=0 on est sur MPP donc pas bouger

!!! MPP change beaucoup en fonction de la lum

comment prendre en compte ca: compare avec une variation de tension en dessous et au dessus de celle précédente et on se dirige juste vers le P le plus grand

si dP new > dP old , alors le dV doit etre plus grand ( à voir de cmb)

vitesse de basculement des transistors:

Nmos BSS138: td(on)+tr+td(off)+ tf  (max)= 5+18+36+14= 73ns= >13,6986 MHz

Pmos FQB22P10TM: td(on)+tr+td(off)+ tf  (max)=45+350+130+230 =755ns =>1,3245 MHz

Donc Fmax

INA 219 caract:

Perr% =0,5-1%

Verr=+- 50-100uA (&0,1-0,5uA/°C)

Les équations au dessus sont la pour savoir la vitesse de basculement max que l'on peut avoir pour le nmos et pmos afin de ne pas avoir le temps de monter ou descendre à la bonne tension.

Cela sera utile pour la pwm ( étant en developpement)

11/04:

recup adresse des 4 sensors avec ce code: [Detecteur adresse capteur DS18B20](Simon perso\Detecteur_adresse_DS18B20)

pwm seulement avec analogwrite : trop de complication car pas d'utilisation de ledc à partire de la V3 d'arduino...

finir carte n°2 de laura et mel avec les changements à effectuer sur la carte, un checking des connexions et un test de téléversage du code controle_all_esp32 directement sur la carte.

faire 2 connecteurs xt60 female male connexion pour les 2 cartes afin de se connecter de manière plus sécurisé sur la carte.

création d'un wifi (chauffez-les-BEES-) sur la carte avec page associée(192.168.1.2), sans mdp

affichage dynamique des val ina219 et de la température toutes les 3.1sec, voir photo correspondante: [Page Web screenshot](Simon perso\Screenshot_Simon\Page html d'affichage des donées via wifi direct.png)(l'affichage sur le terminal reste le même)

bilan des taches à accomplir:

il manque encore beaucoup d'étapes si l'on veut que le projet soit considéré comme fini.

Pour ma part , je n'ai pas testé la pwm avec une charge sur les  2sorties du montage ( j'ai quand même validé comme quoi nous avions bien un signal périodique de fréquence étant dans les kilo hertz.)

l'affichage des données sur le web peut etre bine plus ammélioré, notamment avec une image en arrière plan ( pas le cas ici car nous n'avons pas une image chargée localement  mais avec internet). Une mise en page des données plus intéressante visuellement. des boutons pour augmenter dynamiquement la température max et la température du cycle d'hystérésis.
