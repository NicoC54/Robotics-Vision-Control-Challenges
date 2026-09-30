C'est une excellente initiative. Faire le bilan de ses erreurs est la meilleure façon de progresser en C++. Votre parcours sur cet exercice résume parfaitement les "douleurs de croissance" classiques quand on passe du C++ basique au C++ avancé (multithreading, pointeurs intelligents, OpenCV).

Voici votre "Cheat Sheet" des erreurs rencontrées, classées par thèmes pour ne plus les refaire :

### 1. Pièges du Multithreading et de la Portée (Scope)

* **L'isolement des threads :** Déclarer le `std::mutex` et l'image partagée dans le `main()` rendait leur accès impossible pour les autres fonctions.
* *Règle :* Les threads ne partagent rien par défaut. Pour partager des données, il faut soit utiliser des variables globales (ce que vous avez fait), soit utiliser une classe (PerceptionPipeline) ou passer des pointeurs/références en arguments.


* **Le Shadowing (masquage de variable) :** Écrire `cv::Mat image_passation = temporaire.clone()` dans un bloc `if` a créé une *nouvelle* variable locale qui a été détruite à la ligne suivante, laissant la vraie variable globale vide.
* *Règle :* Quand on modifie une variable globale existante, on ne remet jamais son type (`cv::Mat`) devant.


* **L'oubli de la boucle infinie :** Sans `while(1)`, vos threads s'exécutaient une seule fois de haut en bas puis mouraient.
* *Règle :* Un thread de perception (caméra, capteurs) nécessite toujours une boucle continue.



### 2. Gestion de la Mémoire et de l'Héritage

* **La copie interdite des `unique_ptr` :** Tenter de faire `for (std::unique_ptr<Filter> filter : filters)` demandait au programme de copier un pointeur "unique", ce qui est interdit par conception en C++.
* *Règle :* On parcourt toujours un conteneur de `unique_ptr` par référence avec `&` (ex: `for (const auto& filter : filters)`).


* **Le destructeur non virtuel :** Au début, votre classe mère `Filter` n'avait pas de destructeur virtuel.
* *Règle :* En C++, si une classe possède au moins une méthode virtuelle (`applyFilter`), elle **doit** avoir un destructeur virtuel (`virtual ~Filter() = default;`), sinon la mémoire des classes filles (Gauss, Canny) ne sera pas libérée proprement.



### 3. Utilisation des ressources matérielles (OpenCV)

* **Déclarer la caméra dans la boucle :** Placer `cv::VideoCapture cap(0);` à l'intérieur du `while(1)` demandait au système d'allumer le matériel USB de la caméra des milliers de fois par seconde.
* *Règle :* On allume (initialise) le matériel *avant* la boucle, on le lit (capture) *pendant* la boucle.


* **La signature des fonctions OpenCV :** L'oubli du nom de fenêtre dans `imshow(image)` ou l'invention de la fonction `keyOff()`.
* *Règle :* C'est toujours `cv::imshow("Nom de fenêtre", image);` et la boucle vidéo a un besoin absolu de `cv::waitKey(1);` pour laisser à l'OS le temps de dessiner l'image.



### 4. Syntaxe C++ pure (Attention aux détails)

* **Déclaration du côté droit :** `Capture_Image >> cv::Mat temporaire;`
* *Règle :* En C++, on déclare toujours la variable sur une ligne (ou à gauche), on ne peut pas déclarer un type à droite d'un opérateur de flux.


* **Confusion Noms de Fonctions / Variables :** Vous avez essayé d'assigner une image au nom de votre propre fonction (`Capture_Image = temporaire.clone();`).
* *Règle :* Donnez des verbes aux fonctions (`captureImage()`) et des noms aux variables (`captured_image`) pour ne plus jamais les confondre.


* **Sensibilité à la casse (Majuscules/Minuscules) :** *C'est d'ailleurs la toute dernière erreur qui reste dans votre code actuel (ligne 102) !* Vous avez déclaré `cv::Mat Captured_image;` (avec un **i** minuscule) mais vous essayez d'écrire dedans via `Captured_Image = image_passation.clone();` (avec un **I** majuscule). Le C++ ne pardonnera pas cette différence.