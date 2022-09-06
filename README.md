La plaque tournante PECO est livrée sans motorisation. L’idée est de motoriser et d’automatiser les déplacements de la plaque. Pour permettre à chacun d’adapter le montage à sa situation, j’ai choisi de mémoriser dans l’EEPROM de l’Arduino la position de chacune des voies, puis de gérer les déplacements de la plaque en fonction des emplacements mémorisés.

Gestion de plaque tournante PECO avec un Arduino Nano
Connecter l'Arduino Nano

Pour réaliser ce montage à base d'Arduino, il serait bon d'avoir quelques bases en Arduino (connexion, programmation avec l'IDE...). Éventuellement reportez vous à l'article de Christophe, paru il y a quelques années sur le blog. On a aussi besoin de savoir souder (voir ici dans les ebook), et ne pas être trop maladroit de ses mains.

Au démarrage l’Arduino Nano fait tourner un programme de test qui fait clignoter la LED interne. Connectez l'Arduino Nano au PC ou au Raspberry Pi. Sur PC aller dans gestionnaire de périphériques

L'Arduino Nano est sur le port COM9. Réglez le type de carte et le port dans le menu Outils. Téléverser un programme vide, le clignotement de la LED s'arrête.

#Composants

Après une réflexion et une comparaison entre différents modèles d’Arduino et le Raspberry Pi PICO, j’ai opté pour un Arduino Nano qui dispose du nombre d’Entrées/Sorties suffisant. Voici les composants mis en œuvre dans ce montage. Ils proviennent de chez Amazon et Aliexpress (les liens sont pour certains des liens affiliés). Les prix peuvent varier en fonction du cours du dollar.

Alimentation à découpage
<img src="https://www.framboise314.fr/wp-content/uploads/2022/09/Arduino_nano_PECO_08.jpg">
C'est cette alimentation qui sera chargée de fournir l'énergie à l'ensemble du montage. Le moteur consomme au maxi 1A/phase, on ajoute l'Arduino, les LEDs... soit quelques dizaines de mA. J'ai donc opté pour une alimentation 12v capable de fournir 2A. Elle est vendue une dizaine d'euros. L'alimentation possède un potentiomètre de réglage, il faudra vérifier que la tension de sortie est bien réglée à 12V.

#Driver A4988

Le driver A4988 va se charger de piloter le déplacement du moteur pas à pas. 3 entrées permettent de programmer le nombre de pas par tour. Il s'alimente en 5v pour la partie logique et ici en 12 volts pour la commande du moteur. Il coûte environ 2€ frais de port compris.




DIP Switch x4

Le DIP switch ce sont simplement 4 petits interrupteurs à glissière. J'en utilise 3 pour paramétrer le nombre de pas du A4988 ci-dessus, il en reste un qui sert à passer de mode programme à mode normal. On peut utiliser ce genre d'interrupteur car on n'y accède que rarement. Ce genre d'interrupteur est vendu ~8€ pour 10 pièces.

Carte prototype et borniers à vis

Pour monter le A4988 et le DIP switch, j'ai opté pour une carte prototype de 7x3 cm. Ça ne valait pas le coup pour un montage unitaire de développer une carte de circuit imprimé (PCB) dédiée. J'ai trouvé un lot de cartes prototypes de diverses dimensions qui me serviront pour d'autres montages, mais vous pouvez trouver ce genre de cartes en plus petites quantités. Le lot de cartes prototypes avec les borniers à vis est vendu 20€.

Alimentation 5 volts

Pour alimenter l'Arduino Nano et la logique du A4988 j'ai opté pour ce modèle d'alimentation à découpage à base de LM2596. Il dispose d'un potentiomètre 10 tours pour un réglage fin de la tension de sortie. Vous règlerez la tension de sortie à 5,1v avant de connecter quoi que ce soit à cette alimentation. Je les ai trouvés à 12€ les 6 modules. Il y a du rab mais ça sert toujours ce genre de petite alim à découpage.

Arduino Nano avec bornier à vis

Pour l'Arduino Nano, vu le nombre de connexions à prévoir, je ne voulais pas me lancer sur du câblage soudé sur une carte prototype. Il existe des borniers à vis qui simplifient le câblage, tout en le sécurisant. De plus les noms des E/S sot rappelés devant chaque borne à vis, ce qui réduit le risque d'erreurs. On trouve l'ensemble autour de 7 euros.



Moteur pas à pas NEMA17

Le
 moteur NEMA17 est un classique, il comporte 2 bobines et consomme 1A par phase. C'est le modèle "galette" peu épais et avec un couple réduit mais largement suffisant pour faire tourner la plaque tournante PECO même avec moins de 1A sur chaque bobine. On le trouve pour 13€ environ.

Fourchette optique

La fourchette optique est chargée de détecter le passage de la languette entrainée par le moteur pas à pas. Cela permet de fixer (arbitrairement) une position ZERO qui servira de référence par la suite. Cette fourchette sert à chaque démarrage et je l'utilise aussi en fonctionnement normal car le faisceau ne doit pas être coupé sinon ça provoque l'arrêt et la mise en sécurité (voir plus loin). Ce modèle est fourni avec le câble et coûte 7€ les 5 capteurs. C'est un modèle qui est utilisé sur des imprimantes 3D.

Coupleur 5mm/4mm

Comme je voulais quelque chose de propre, j'ai opté pour un coupleur en laiton (oui, je sais j'aurais pu l'imprimer en 3D...) mais ça fait quand même plus professionnel, non ? Côté moteur NEMA17 on a un trou de 5mm et un blocage par vis pointeau. Côté plaque tournante on a un trou de 4mm qu'il faudra peut être ajuster un peu car l'axe de la PECO fait 4,1mm... On sortira la lime ;-) Le lot de coupleurs  coûte 8€ (indisponible au moment où j'écris ces lignes).

Ecran OLED 0,91 pouce

Ceux qui me suivent savent que j'aime les LEDs. On peut apprendre plein de choses avec un CPU, un MCU, une LED et un bouton poussoir. Pareil avec ce petit écran facile à mettre en œuvre et qui se prend en main rapidement. Il ne coûte que 8€ les 2 pièces.

Buzzer

"Faites du bruit..." c'est le rôle du buzzer, parfois même un peu trop bruyant ! Pour les tests j'ai du mettre un ruban adhésif sur le trou du buzzer pour atténuer le bruit, suite aux plaintes de mon entourage :lol: Ce module est vendu 8€ pour les 5 pièces.

Boutons poussoirs

J'avais commandé ces boutons poussoirs pour un autre projet, comme il m'en restait je les ai utilisés ici. Ils sont munis de fils que j'ai dessoudés pour utiliser les boutons poussoirs sur le tableau de commande. Ils sont vendus par 14 au prix de 9,5€.

Inverseur astable

Cet interrupteur est "astable", il rev
ient au milieu après qu'on ait appuyé vers la droite ou la gauche (ou le haut et le bas si vous le montez verticalement :-)  ). Je l'utilise pour déplacer le plateau dans un sens ou dans l'autre lors de la programmation. Les 5 interrupteurs sont vendus 5€.

Interrupteur A/M

Celui-c
i c'est l'interrupteur Arrêt/Marche. J'ai utilisé ce modèle que j'avais en stock, il a une position centrale stable et deux positions latérales stables également. Il permet en laissant la tige au centre de n'alimenter aucune sortie. Il vaut 9€ pour les 12 pièces.




Condensateur

Le condensateur est chargé de supprimer les impulsions électriques transitoires générées par la commutation du driver A4988 sur les bobines du moteur pas à pas. Il est fortement conseillé par le fabricant du driver. Ici j'ai doublé le condensateur. On n'est jamais trop prudent.  Le condensateur vaut 1€ pièce, vendu par lot de 5.

Montage
La carte principale
Schéma




La carte est basée sur un Arduino Nano. Comme de nombreux microcontrôleurs, les broches de celui-ci peuvent avoir plusieurs fonctions sélectionnables par programme. La sortie D13 est connectée à la LED interne, il faut éviter de l'utiliser si possible. Les entrées analogiques peuvent être utilisées en mode numérique (0/1) sauf ADC6 et ADC7 qui ne peuvent être que des entrées an logiques.



Cliquez pour agrandir

A gauche on trouve la fourchette optique, dont la coupure du faisceau infra rouge provoque une interruption sur l'Arduino. Le programme principal s'arrête et l'Arduino exécute l'interruption qui doit être un programme court. Ici on met juste un flag (drapeau) à 1 pour signaler qu'il y a eu une interruption.

Le cœur de la carte est bien entendu l'Arduino Nano qui pilote l'écran OLED via le bus I2C. Il commande aussi le driver de moteur pas à pas A4988 en lui envoyant une information de direction (sens horaire/anti-horaire) et des impulsions pour le faire avancer d'un pas par impulsion. C'est lui qui pilote les LED situées au dessus des boutons poussoirs (BP) et la LED LOCK qui indique soit la mise en mémoire d'une valeur (flash) soit une erreur et le verrouillage du programme (LOCK allumé en continu = moteur arrêté). Il pilote également le buzzer que j'utilise pour certaines opérations, comme l'initialisation ou l'enregistrement des données en mémoire. Il sert à confirmer que l'opération s'est bien passée.

DIP Switch

Le A4988 est connecté à un DIP Switch (ci-dessus). Les interrupteurs 1,2 et 3 sont reliés aux entrées MS1 à MS3 du driver. Les pattes sont tirées à la masse par une résistance de pull-down, ce qui fait que si on laisse MS1 à MS3 en l'air, on est dans le cas OFF/OFF/OFF et qu'il y a 200 pas par tour. Si le switch est sur ON, il envoie du +5v (un 1) sur l'entrée correspondante. Dans le cas présent j'ai réglé le switch sur 1/8 de pas ce qui fait 1600 pas par tour et une précision de 360/1600 = 0.225° par pas.

Moteur Pas à Pas


Cliquez pour agrandir




Driver A4988

Les 4 broches du A4988 situées en haut à droite sont reliées au moteur pas à pas, les couleurs correspondent aux couleurs des fils du moteur. L'alimentation du moteur se fait en 12 volts (le driver A4988 nécessite de 8 à 35 volts pour fonctionner). Vous pouvez consulter la notice du A4988 en cliquant sur ce lien. Le potentiomètre permet de régler le courant qui circule dans les fils du moteur PAP. La formule permettant de calculer le corant en fonction de la tension mesurée sur le curseur du potentiomètre est à ajuster en fonction des résistances shunt montées sur la carte. Sur ma carte ce sont de 0,1Ω. La formule est




VREF = 8 x Imax x RCS  voir ici pour plus de détails. VREF est la tension sur le curseur du potentiomètre, Imax le courant maxi que vous autorisez dans le moteur PAP et RCS la résistance de shunt montée sur la carte. Si vous mettez trop de courant, comme le moteur est maintenu en position à l'arrêt, il va chauffer. C'est normal qu'il chauffe mais si le courant est trop fort... il devient vraiment brulant. Si vous réglez le courant trop bas, le moteur risque de perdre des pas pendant sa rotation. A vous de faire au mieux.

Le switch 4 est lu seulement au lancement du programme. s'il est sur ON, le logiciel passe en MODE PROGRAMME qui permet d'enregistrer dans la mémoire de l'Arduino les positions des voies. Sur cette réalisation il y a 4 voies mais il est très facile d'étendre le programme avec un grand nombre de voies.

L'alimentation 5 volts reçoit en entrée le 12 volts de l'alimentation à découpage et le transforme en 5 volts pour alimenter l'Arduino et la logique du driver A4988.

Photos


Cliquez pour agrandir

J'ai réalisé une plaque imprimée en 3D pour supporter l'ensemble de la carte de commande. A gauche l'alimentation à découpage, au centre la carte du driver A4988 et à droite l'Arduino Nano monté sur sa carte de borniers à vis.

La carte d'alimentation 5 volts est logée SOUS la carte du driver A4988. En raison de la proximité des deux cartes, j'ai ajouté un intercalaire d'isolation imprimé en 3D.

Le moteur et sa remise à zéro


Cliquez pour agrandir

Le moteur pas à pas est un NEMA17 à 4 fils (2 bobines). Au démarrage du programme (en mode NORMAL ou en mode PROGRAM), le moteur tourne rapidement dans le sens inverse des aiguilles d'une montre (oui, je sais il n'y a plus beaucoup de montres avec des aiguilles :-(  ), disons vers la gauche... Lorsque la languette coupe le faisceau infrarouge (IR) une interruption est déclenchée et le moteur s'arrête. Pour confirmer de façon certaine la position zéro, le moteur repart vers la droite (sens horaire) sur une faible distance, juste pour libérer le faisceau IR. Il ramène alors la languette très lentement jusqu'à ce qu'elle coupe le faisceau IR. C'est cette position qui est appelée 0 (zéro) et sert ensuite de référence. Vous observerez le même système de prise de zéro sur de nombreuses machines de makers et industrielles (découpe laser, impression 3D...).

Sur cette vue dessus, on voit les trois "pieds" destinés à assurer la fixation de ce module sur la plaque tournante PECO. La pièce en laiton montée sur l'axe du moteur est le coupleur.

Le panneau de commande
Schéma


Cliquez pour agrandir

Le schéma de cette partie est relativement simple. A gauche l'interrupteur A/M qui coupe le secteur. Lors du câblage pensez à bien isoler et protéger ces connexions car le 230v peut être mortel par simple contact. Soignez les soudures et protégez les (gaine thermorétractable, ruban adhésif d'électricien...).

On trouve ensuite 2 LEDs qui indiquent la présence du 12v et du 5v. Cela permet de contrôler le fonctionnement des alimentations et de détecter une panne. La LED Lock indique le verrouillage du moteur en cas d'erreur. C'est une boucle infinie et il faut obligatoirement éteindre l'ensemble pur reprendre la main. Les 5 LEDs rouges sont situées au dessus des BP et s'allument le temps de la rotation du moteur pour rejoindre une voie. Le buzzer permet de vérifier auditivement le bon déroulement des opérations.

L'inverseur G/D est astable, c'est à dire qu'il revient en position médiane quand on le relâche. Il sert à déplacer le moteur dans un sens ou dans l'autre pour amener le rail du plateau tournant en face des rails de l'installation.

Lecture des boutons poussoirs

L'Arduino Nano a pas mal d'Entrées/Sorties mais c'est un peu juste quand même quand il faut lire plusieurs boutons poussoirs, comme les 5 BP de ce montage. On a la possibilité de lire les BP dans une matrice, comme sur les claviers mais ce n'est pas intéressant ici avec juste 5 boutons il faudrait 3 lignes et 2 colonnes pour un maximum de 6 touches. Soit dans notre cas 5 fils... Pour 5 boutons ! Avantage nul :-D

Deuxième solution, celle que j'ai retenue ici, utiliser une entrée analogique et générer une tension avec les BP. Cela permet de lire les 5 BP avec une seule entrée. Inconvénient (bin oui, il y en a toujours!)on ne peut lire QU'UN bouton à la fois alors que la matrice permet de lire des combinaisons de touches. Par exemple regardez ce qui se passe si on appuie sur BP5 et BP1 en même temps... Vous court-circuitez les 4 résistances du milieu et vous envoyez du 2,5v sur A7. L'Arduino pense (enfin, il ne pense rien, c'est votre programme qui pense hein !) que vous avez appuyé sur le bouton 3...

Bon, on regarde comment ça marche ?

L'entrée A7 accepte une tension continue entre 0 et 5 volts et la mesure grâce à un ADC qui convertit la tension en valeur numérique. O volt correspond à ... Zéro et 5 volts donnera une valeur de 1024.  Si vous n'appuyez sur aucun BP, A7 est reliée à la masse par la 100K et renvoie le chiffre 0. Ensuite j'ai fait un pont diviseur avec des résistances de 1K (j'ai pris cette valeur parce que j'en avais plein). J'ai mis a coté de chaque fil la tension qui est récupérée, et la valeur numérique qui sortira du convertisseur ADC. Par exemple 2,5 volts donne 512... La moitié de la tension donne la moitié du nombre maxi (1024). On est pas mal, là...
Et surprise (enfin non) comme tous mes pas sont égaux et multiples de 170, si je divise par 170, j'obtiens... Le numéro du bouton poussoir ! Fastoche, non ?
Donc lorsque j'appuie sur un bouton, la tension sur A7 devient différente de Zéro volt, le convertisseur ADC sort un nombre que le programme récupère. Le programme divise le nombre par 170 (avec un arrondi parce que les valeurs fluctuent un peu) et bingo, on récupère le numéro du BP appuyé.

Photo

Pour la face avant j'ai imprimé cette plaque. Les noms sont en relief mais je les ai doublés en bleu pour que vous localisiez tout ça.

Conception 3D

Les éléments du montage : support moteur, languette optique, face avant, isolant... Sont disponible sur Thingiverse. C'est modélisé avec Fusion 360, je mets les fichiers STL en ligne mais les fichiers archive de Fusion 360 sont disponibles si vous le souhaitez.










Quelques étapes de la modélisation.

Le Programme

Le programme comporte 2 parties : un mode programme (quand le switch program est ON) et un mode normal.

En mode PROGRAM on cale la position du plateau avec l'inverseur D/G. Pour affiner le réglage, on appuie sur le BP 1/2T et le moteur passe en vitesse très lente pour permettre d'ajuster la position.  On valide quand c'est bon, par le bouton poussoir VOIE1, VOIE2, VOIE3 ou VOIE4. La valeur est mémorisée dans l'EEPROM de l'Arduino et sera toujours présente au prochain démarrage de la machine.

En mode NORMAL, le moteur fait sa prise de ZERO (indiqué R.A.Z. sur l'écran) et va se positionner sur la voie 1. Le BP 1/2T permet de faire un demi tour pour retourner la locomotive, les boutons V1 à V4 envoient le plateau sur la voie correspondante. Le plateau ne coupe pas le faisceau IR de la fourchette pour éviter d'enrouler (casser ?) le fil qui alimente les rails du plateau. Si le faisceau est coupé le programme s'arrête et l'écran affiche ERREUR.

J'ai essayé de commenter au maximum le programme pour le rendre compréhensible. Après ce n'est pas un programme de développeur. C'est un programme de maker. Il fait ce pour quoi il est écrit, mais n'est certainement ni optimisé, ni parfait. J'accepterai toutes les critiques, à condition que celui qui critique... améliore le programme en fonction de ses remarque, et le remette à disposition de la communauté :-D

Le programme est disponible sur Github et vous pouvez le télécharger librement. Je n'ai pas encore le plateau et il y aura à coup sûr des modifications qui seront mises en ligne au fur et à mesure.
