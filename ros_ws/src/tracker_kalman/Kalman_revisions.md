Voici la fiche de révision complète et compilée au format Markdown. Vous pouvez copier l'intégralité de ce bloc pour vos notes.

# Fiche de Révision : Architecture et Conception d'un Filtre de Kalman

## 1. La Conception du Vecteur d'État ($X$)

Le vecteur d'état représente le monde mathématique imaginé par l'ordinateur.

* **La Règle du Curseur Cinématique ("Un cran au-dessus") :** Pour qu'un filtre soit prédictif (et non pas juste un lisseur statique), le vecteur d'état doit toujours inclure la dérivée de la variable que l'on souhaite estimer.
* *Exemple :* Si on veut la Position $\rightarrow X = [p, v]^T$. Si on veut la Vitesse $\rightarrow X = [v, a]^T$.
* *Pourquoi ?* Cela donne au filtre la capacité mathématique de comprendre l'inertie et d'anticiper le mouvement entre deux mesures.


* **Le Piège de la Dérive Infinie (Observabilité) :** Ne jamais inclure une variable dans $X$ si elle n'est pas nécessaire à l'objectif final **ET** qu'aucun capteur (dans $Z$) ne peut la corriger directement ou indirectement. Sans correction externe, l'intégration des micro-erreurs fera dériver la variable vers l'infini.

## 2. Le Rôle du Vecteur de Commande ($U$)

Le vecteur $U$ contient les forces ou commandes connues qui agissent directement sur la variable la plus haute de $X$.

* **La Règle de l'Anti-Doublon :** Il est mathématiquement interdit de demander au filtre d'estimer une dynamique dans $X$ si on la lui impose déjà comme une vérité absolue dans $U$.
* *Exemple :* Si $U$ = Accéléromètre, $X$ **ne peut pas** contenir l'accélération. $X = [p, v]^T$.


* **Le Cas du Tracking Externe ($U=0$) :** Lorsqu'on suit un objet externe (ballon, visage, autre voiture) dont on ne connaît ni les commandes internes ni la physique exacte, on pose $U=0$. Le filtre suppose une vitesse constante et déduit de lui-même les variations grâce aux observations ($Z$).
* **L'Exception de l'"Error-State" (ESKF) :** Il est possible d'avoir une variable de même unité dans $U$ et dans $X$ (ex: Vitesse roue dans $U$, Vitesse patinage dans $X$) **uniquement si** la variable dans $X$ ne représente plus la physique du robot, mais l'**erreur du capteur** (biais, glissement). Le modèle devient alors : $État_{nouveau} = État_{ancien} + (U - Erreur) \cdot \Delta t$.

## 3. Les Pièges Mathématiques et Informatiques

* **Le temps Continu vs Discret :**
* *La confusion :* Penser que la matrice de transition $F$ contient des dérivées ($dX/dt = FX$).
* *La réalité :* En informatique (temps discret), l'équation est $X_{k+1} = F \cdot X_k + G \cdot U_k$. La matrice $F$ ne contient aucune dérivée, elle contient uniquement les **additions** et le temps d'intégration $\Delta t$ nécessaires pour passer d'une itération à l'autre ($p_{nouveau} = p_{ancien} + v \cdot \Delta t$).


* **Le Piège du Remplissage de $Z$ (L'erreur des zéros) :**
* *L'erreur :* Ajouter des $0$ dans le vecteur d'observation $Z$ pour qu'il fasse la même taille que $X$ (ex: $Z = [x_{mesuré}, y_{mesuré}, 0, 0]^T$ pour matcher $X = [x, y, v_x, v_y]^T$). Cela force le filtre à croire que la vitesse mesurée est de $0$, bloquant l'objet mathématiquement.
* *La règle :* Le vecteur $Z$ contient **strictement** le nombre de valeurs mesurées physiquement. S'il y a 2 capteurs, $Z$ a 2 lignes.


* **La Matrice $H$ (L'adaptateur) :**
* C'est la matrice $H$ qui se charge de faire le pont de dimension entre le grand monde mathématique ($X$) et le petit monde réel ($Z$). $H$ n'est pas carrée si le nombre de capteurs est inférieur au nombre de variables d'état. L'équation de correction est basée sur l'écart (l'innovation) : $Z - H \cdot X$.



## 4. Architecture Système : $U$ ou $Z$ ? (L'exemple du Rover)

Quand on dispose d'un capteur haute fréquence (ex: encodeur de roues, odométrie) qui est sujet à des erreurs violentes (patinage), deux architectures s'affrontent :

| Stratégie | Méthode | Avantage | Inconvénient |
| --- | --- | --- | --- |
| **Risquée (Mécanisation)** | Mettre le capteur dans $U$. Estimer l'erreur dans $X$. | Consomme très peu de CPU, parfait pour les biais lents (gyroscope). | Si l'erreur est soudaine (patinage), le modèle est instantanément corrompu avant que la caméra ne corrige. |
| **Robuste (Fusion $Z$)** | Poser $U=0$. Mettre le capteur dans $Z$ avec la caméra. | Permet d'ignorer dynamiquement le capteur si une anomalie est détectée (en augmentant son bruit $R$). | Demande un peu plus de calcul matriciel pour le filtre. |

## 5. Mémo ROS 2 : Pipeline Zero-Copy & OpenCV

* **Transport :** Utilisation impérative de `std::unique_ptr` entre les Component Nodes (qui partagent le même espace mémoire) pour éviter toute copie.
* **QoS :** Les nœuds doivent partager le même profil de Qualité de Service (`SensorDataQoS` / Best Effort) sous peine de blocage silencieux des communications.
* **Traitement Image :**
* Bruit visuel $\rightarrow$ Traité par OpenCV (ex: `cv::GaussianBlur`).
* Bruit de mesure (tremblements des détections) $\rightarrow$ Traité par les matrices $Q$ (bruit modèle) et $R$ (bruit capteur) du Filtre de Kalman.
* Accès mémoire $\rightarrow$ Utiliser exclusivement `cv::toCvShare` pour un accès en lecture seule au pointeur d'image (Zero-Copy). Bannir `cv::toCvCopy` qui déclenche une allocation mémoire redondante.



---

## 6. Cas Pratiques : Architectures Classiques en Robotique

Voici les *design patterns* (modèles d'architecture) les plus courants dans l'industrie pour choisir $X$ et $U$.

### 🤖 Cas A : Le Robot Mobile Différentiel (Aspirateur autonome, TurtleBot)

* **L'objectif :** Suivre la position $(x, y)$ et le cap $(\theta)$ du robot sur une carte 2D.
* **Le vecteur d'état $X$ :** $[x, y, \theta, v, \omega]^T$
*(Position X, Position Y, Cap, Vitesse linéaire, Vitesse de rotation).*
* **La commande $U$ :** $[v_{cmd}, \omega_{cmd}]^T$ (Les consignes envoyées aux moteurs).
* **Pourquoi ?**
* *Le Curseur :* On veut $[x, y, \theta]$, on monte d'un cran en ajoutant les vitesses $[v, \omega]$ car la cinématique d'un robot à roues les lie mathématiquement.
* *Le Choix de U :* On pilote le robot ! L'ordinateur connaît exactement les commandes qu'il envoie aux contrôleurs des moteurs. On utilise donc ces commandes comme "force motrice" dans $U$ pour prédire la nouvelle position, tandis que le Lidar ou la caméra corrigeront dans $Z$.



### 🏭 Cas B : Le Bras Robotique Industriel (Axe par axe)

* **L'objectif :** Contrôler l'angle exact d'une articulation lourde (ex: coude du robot soudeur) avec une fluidité parfaite pour ne pas casser la mécanique.
* **Le vecteur d'état $X$ :** $[\theta, \omega, \alpha]^T$
*(Angle, Vitesse angulaire, Accélération angulaire).*
* **La commande $U$ :** $[Jerk_{cmd}]$ (La commande de secousse envoyée au variateur industriel).
* **Pourquoi ?**
* *Le Choix de U :* Dans l'industrie lourde, on ne commande jamais un moteur brutalement en vitesse (pour éviter les à-coups). On envoie une consigne de *Jerk* (la dérivée de l'accélération).
* *Le Curseur :* Puisque $U = Jerk$, on doit remplir le vecteur $X$ avec toutes les dérivées inférieures jusqu'à notre objectif final (l'Angle $\theta$). D'où le trio Angle/Vitesse/Accélération.



### 🚁 Cas C : Le Drone FPV (Stabilisation de l'Attitude)

* **L'objectif :** Connaître l'inclinaison 3D exacte du drone (Roulis $\phi$, Tangage $\theta$, Lacet $\psi$) sans que l'horizon artificiel ne dérive.
* **Le vecteur d'état $X$ :** $[\phi, \theta, \psi, \text{Biais}_x, \text{Biais}_y, \text{Biais}_z]^T$
*(Les 3 angles, et les 3 erreurs du gyroscope).*
* **La commande $U$ :** $[\omega_x, \omega_y, \omega_z]^T$ (Les 3 vitesses angulaires lues par le gyroscope à 1000 Hz).
* **Pourquoi ?**
* *Le Choix de U :* C'est l'architecture **Error-State (ESKF)**. Le capteur inertiel (gyroscope) est tellement rapide qu'on s'en sert comme "moteur de prédiction" dans $U$.
* *L'Anti-Doublon :* Puisque la vitesse angulaire est dans $U$, on **n'a pas le droit** de la mettre dans $X$.
* *La parade :* À la place, on demande au Kalman d'estimer les *Biais* (défauts de fabrication du capteur) dans $X$. L'accéléromètre (qui regarde la gravité terrestre) servira de juge dans $Z$ pour corriger ces biais.



### 🎯 Cas D : La Tourelle de Ciblage (Suivi de cible externe)

* **L'objectif :** Suivre un drone ennemi ou une balle de tennis en vol pour orienter une caméra vers lui.
* **Le vecteur d'état $X$ :** $[x, y, z, v_x, v_y, v_z]^T$
*(Position 3D et Vitesse 3D).*
* **La commande $U$ :** $0$ (Vecteur nul).
* **Pourquoi ?**
* *Le Choix de U :* L'objet ciblé est indépendant. Nous n'avons pas accès à son ordinateur de bord ni à ses capteurs. Il est donc impossible de remplir $U$.
* *La stratégie :* On pose $U=0$ (Modèle à vitesse constante). C'est le bruit de processus (la Matrice $Q$) qui absorbera les accélérations surprises de la cible, et la mesure (Radar/Caméra dans $Z$) qui recadrera la prédiction.



### 🚗 Cas E : Le Régulateur de Vitesse Adaptatif (Voiture sur autoroute)

* **L'objectif :** Maintenir une distance de sécurité avec la voiture de devant, sans freiner ni accélérer de façon saccadée.
* **Le vecteur d'état $X$ :** $[\text{Distance}, v_{relative}, a_{relative}]^T$
*(Distance qui nous sépare, Vitesse relative, Accélération relative).*
* **La commande $U$ :** $[0]$ (Ou l'accélération de notre propre voiture si on la connaît très précisément).
* **Pourquoi ?**
* *L'optimisation :* La position absolue sur la Terre (GPS) n'a aucune importance ici. On conçoit un vecteur d'état purement **relatif**.
* *Le Curseur :* On met l'accélération relative dans $X$ car une voiture devant nous met du temps à freiner (inertie). Avoir la composante $a_{relative}$ permet au filtre de voir que la voiture de devant *commence* à freiner avant même que la distance ne se réduise dangereusement. Le Radar de pare-choc ira dans $Z$.