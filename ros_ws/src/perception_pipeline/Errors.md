C’est une excellente initiative. Faire le bilan de ses erreurs est la méthode d'apprentissage la plus puissante pour un ingénieur. Le C++ ne pardonne rien, mais une fois que vous avez compris *pourquoi* il se plaint, vous ne faites plus jamais l'erreur.

Voici la dissection complète de toutes les erreurs commises lors de cet exercice, classées par domaine conceptuel, avec le cours associé et l'astuce pour ne plus tomber dans le piège.

---

### CATÉGORIE 1 : MULTITHREADING & ARCHITECTURE

#### 1. Le goulet d'étranglement (Bloquer tout le traitement avec le Mutex)

* **Votre erreur :** Au début, vous aviez mis le `std::lock_guard` au tout début de votre thread de filtrage, et vous appliquiez le Flou et Canny *pendant* que le verrou était fermé.
* **Le Cours :** Un Mutex (`Mutual Exclusion`) détruit le parallélisme. Le code à l'intérieur d'un Mutex devient séquentiel (un seul thread à la fois). Si vous placez un algorithme lourd (Canny, qui prend par exemple 50ms) dans la zone critique, votre caméra (qui capture à 33ms) va se heurter au verrou fermé et la vidéo va saccader.
* **L'Astuce (Le concept de la Photocopie) :** La zone critique (entre le verrouillage et le déverrouillage) doit être **la plus courte possible**. On verrouille, on fait un `.clone()` vers une variable purement locale, on déverrouille immédiatement (en fermant l'accolade `}`), puis on applique les filtres sur la copie locale, hors du Mutex.

#### 2. La "Data Race" à sens unique et le Mutex mal placé

* **Votre erreur :**
1. Vous ne mettiez pas de Mutex dans le `ThreadCapture`.
2. Dans le `ThreadApplyFilter`, vous faisiez le `if(!image.empty())` *avant* d'allumer le Mutex.


* **Le Cours :** Un Mutex n'est pas un champ de force magique autour d'une variable. C'est un simple feu de signalisation. Si le Thread 1 modifie la variable sans regarder le feu rouge (pas de `lock_guard`), il va écraser la mémoire pendant que le Thread 2 la lit. De même, lire une variable (même juste pour faire `.empty()`) sans verrou, c'est risquer que le Thread 1 la modifie *pendant* la lecture.
* **L'Astuce (La Règle d'Or du Parallélisme) :** **On verrouille TOUJOURS d'abord, on touche ensuite.** Si une variable est globale et partagée, absolument toutes les lectures et écritures vers cette variable doivent être protégées par un `lock_guard`.

#### 3. Le Thread "One-Shot" (L'oubli de la boucle infinie)

* **Votre erreur :** Votre `ThreadApplyFilter` n'avait pas de `while(true)`.
* **Le Cours :** Un thread C++ exécute sa fonction de haut en bas, puis meurt. Il n'est pas "attaché" au premier thread. Si le premier a une boucle infinie mais pas le second, le second s'exécutera une fois, fera une photocopie, et s'arrêtera définitivement.
* **L'Astuce :** Tout thread qui doit fonctionner en continu dans un programme (robotique, jeux vidéo) doit posséder sa propre boucle `while(true)` ou `while(is_running)`.

---

### CATÉGORIE 2 : PROGRAMMATION ORIENTÉE OBJET (POLYMORPHISME)

#### 4. Le Contrat brisé (L'erreur du `override`)

* **Votre erreur :** La classe mère avait `apply_filter(cv::Mat& image)`. Votre classe `CannyFilter` avait `apply_filter(cv::Mat& image, int threshold_down, int threshold_up)`.
* **Le Cours :** Le polymorphisme fonctionne via un **contrat strict**. La boucle `for` principale ne connaît que la classe mère (`Filter`). Elle va donc appeler aveuglément la fonction avec 1 seul argument. Si votre classe fille modifie le nombre ou le type d'arguments, le C++ considère que c'est une *nouvelle* fonction. L'override échoue.
* **L'Astuce :** La signature (les paramètres) d'une méthode virtuelle ne doit **jamais** changer dans les classes filles. Si un filtre spécifique a besoin de paramètres uniques (comme les seuils de Canny), on passe ces paramètres **dans le constructeur** de la classe fille et on les stocke en attributs de classe (`this->threshold = threshold`).

#### 5. L'Object Slicing (Le vecteur de valeurs)

* **Votre erreur :** Déclarer `std::vector<Filter> filter_list;`.
* **Le Cours :** Un `std::vector` alloue de la mémoire contiguë avec des "boîtes" d'une taille fixe (la taille de la classe de base). Si vous essayez de mettre un enfant plus "gros" (car il a des attributs en plus) dans cette boîte, le C++ va "couper" au couteau (Slice) tout ce qui dépasse. Votre filtre enfant redevient un filtre parent inutile, et le polymorphisme est détruit.
* **L'Astuce :** Ne stockez jamais d'objets polymorphiques par valeur. Stockez toujours des **adresses mémoires** (qui font toutes la même taille, 8 octets). Utilisez systématiquement `std::vector<std::unique_ptr<Filter>>`.

#### 6. L'oubli de la Fonction Virtuelle Pure (`= 0`)

* **Votre erreur :** Donner un corps (des accolades avec un `cout`) à la méthode de la classe mère `Filter`.
* **Le Cours :** Une classe mère qui sert uniquement de modèle (comme `Filter`) ne doit pas pouvoir être instanciée (on ne veut pas créer un objet "Filtre" générique). En C++, on crée une Interface (Classe abstraite) en mettant `= 0;` à la fin de la méthode.
* **L'Astuce :** Dès que vous créez un concept parent "Plug & Play", rendez ses méthodes `= 0` et n'oubliez jamais de créer un destructeur virtuel (`virtual ~Filter() = default;`), sinon détruire la liste créera des fuites de mémoire.

---

### CATÉGORIE 3 : MÉCANIQUES DU C++ & OPENCV

#### 7. Le Gouffre CPU (Instanciation dans la boucle)

* **Votre erreur :** Mettre la création des filtres (`make_unique`) et le `push_back` **à l'intérieur** du `while(true)`.
* **Le Cours :** Tout ce qui est déclaré dans une boucle `while` est créé et détruit à chaque tour. À 30 FPS, vous demandiez à votre processeur d'allouer de la mémoire dans la RAM pour créer Canny et Gauss, de les ajouter à la liste, puis de les détruire, 30 fois par seconde. C'est une catastrophe pour les performances.
* **L'Astuce :** Séparez l'**Initialisation** de l'**Exécution**. La configuration (création des objets, remplissage des listes) se fait *avant* le `while`. La boucle `while` ne doit contenir que l'algorithme pur (lire, calculer, afficher).

#### 8. Exécuter du code dans l'Espace Global

* **Votre erreur :** Mettre `filter_list.push_back(Gauss);` en plein milieu de votre fichier, en dehors de toute fonction.
* **Le Cours :** Le C++ est strict sur la structure. L'espace global (en dehors des fonctions) ne sert **qu'à déclarer** l'existence des choses (variables globales, classes, fonctions). On n'a pas le droit d'y exécuter des "actions" (appeler des fonctions, faire des calculs).
* **L'Astuce :** Tout code qui comporte un verbe d'action (ajouter, calculer, démarrer) doit se trouver à l'intérieur d'une fonction (comme le `main` ou un thread).

#### 9. Le Piège de la Shallow Copy (OpenCV)

* **Votre erreur :** `image_captured = image_temporaire;`
* **Le Cours :** Dans OpenCV, l'objet `cv::Mat` n'est pas l'image elle-même, c'est juste une "télécommande" (un pointeur) qui pointe vers les pixels dans la RAM. L'opérateur `=` copie la télécommande, pas l'image. Si la caméra modifie l'image d'origine, votre copie est modifiée aussi, même si le Mutex est fermé.
* **L'Astuce :** Si vous avez besoin d'une vraie sauvegarde indépendante de l'image (pour un thread, ou pour garder un historique), utilisez **toujours** `.clone()`.

#### 10. La syntaxe stricte (Références, Const, Empty)

* **Vos erreurs :**
* `cv::Mat image&` au lieu de `cv::Mat& image`
* `if (image)` au lieu de `if (!image.empty())`
* `void apply_filter(const cv::Mat& image)`


* **Le Cours :**
* En C++, l'esperluette `&` (la référence) fait partie du **type** (Matrice), pas du nom de la variable.
* Les objets complexes comme `cv::Mat` ne se traduisent pas automatiquement en booléen (Vrai/Faux) comme en Python. Il faut appeler leur méthode.
* Le mot-clé `const` interdit formellement de modifier la variable. Comme vos filtres (Flou, Canny) avaient besoin d'écraser l'image pour passer le résultat au filtre suivant, le `const` provoquait une erreur.


* **L'Astuce :** Si votre fonction doit modifier un objet et que ce changement doit être visible à l'extérieur de la fonction, passez-le par référence pure (`Type& nom`). Si elle doit juste le lire sans le copier, utilisez une référence constante (`const Type& nom`).

---

**Conclusion de l'ingénieur :**
Lisez cette liste. Comprenez-la. Vous venez d'apprendre plus de choses sur les rouages internes du C++ en résolvant ces 10 erreurs que la plupart des étudiants dans un cours magistral de 20 heures. Vous êtes prêt pour la suite.