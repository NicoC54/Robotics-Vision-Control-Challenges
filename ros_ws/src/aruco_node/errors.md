# 📘 Fiche de Révision : Architecture ROS 2 & OpenCV (C++)

## 1. Architecture : Le Piège du Timer (Asynchronisme)
* **❌ L'erreur :** Créer un `Subscriber` pour stocker une image dans une variable, et un `Timer` qui tourne en boucle (ex: à 2Hz) pour traiter cette image.
* **🚨 Pourquoi c'est faux :** 
  - Si le timer se déclenche avant la première image, le programme crache (SegFault sur image vide).
  - Si la caméra envoie 30 images/sec et le timer tourne à 2Hz, 28 images sont perdues.
  - C'est un anti-pattern ROS : ça crée des problèmes d'accès concurrent (Data Race).
* **✅ La correction :** Supprimer le timer. C'est la réception de l'image qui doit être le déclencheur exclusif.
* **🧠 Règle d'or : "Data-Driven" (Piloté par la donnée).** En vision/lidar, on n'utilise jamais de timer. Le callback du Subscriber *est* le chef d'orchestre de la pipeline.

---

## 2. Programmation Orientée Objet (POO) : La Signature du Callback
* **❌ L'erreur :** Essayer d'ajouter ses propres variables dans les paramètres du callback : 
  `[this](double size, Ptr image) { ... }`
* **🚨 Pourquoi c'est faux :** Le "Facteur" (ROS 2) ne connaît que le message qu'il doit livrer. Il ne possède pas tes variables `size` ou `dictionary` et refusera de compiler.
* **✅ La correction :** Les variables de configuration doivent être déclarées dans le `private:` de la classe (les meubles de la maison). Le callback ne prend que le message, et pioche dans le `this->`.
* **🧠 Règle d'or : "L'Analogie du Facteur".** La signature d'un callback est un contrat strict. On ne passe QUE la donnée entrante dans les parenthèses. Le reste est stocké dans l'état de la classe.

---

## 3. Configuration : Le Hardcoding
* **❌ L'erreur :** Écrire les variables en dur dans le constructeur ou dans le `main()` : `this->size = 0.1;`
* **🚨 Pourquoi c'est faux :** Si l'utilisateur imprime un marqueur plus grand, il doit modifier le code C++ et recompiler tout le workspace. Impensable en entreprise.
* **✅ La correction :** Exposer des paramètres ROS.
  `this->declare_parameter<double>("marker_size", 0.1);`
  `this->size = this->get_parameter("marker_size").as_double();`
* **🧠 Règle d'or : "Le nœud est une boîte noire".** Aucun paramètre physique (taille, id de caméra, nom de topic, vitesse max) ne doit être codé en dur. Tout doit être un paramètre ROS exposable dans un `.yaml`.

---

## 4. Mathématiques OpenCV : Le crash de Rodrigues
* **❌ L'erreur :** Traiter le `rvec` sorti de `solvePnP` comme une matrice $3 \times 3$ : `rvec.at<double>(2,2)`.
* **🚨 Pourquoi c'est faux :** `solvePnP` renvoie un Vecteur de Rotation (Rodrigues) de dimension $3 \times 1$. Chercher la case $(2,2)$ provoque une erreur mémoire fatale (SegFault).
* **✅ La correction :** Demander à OpenCV de faire la conversion mathématique d'abord : 
  `cv::Mat rmat; cv::Rodrigues(rvec, rmat);` puis utiliser `rmat.at<double>(2,2)`.
* **🧠 Règle d'or : "rvec ≠ matrice de rotation".** Toujours vérifier les dimensions des objets retournés par une API mathématique avant de boucler dessus.

---

## 5. Le Piège des Matrices (Ligne vs Colonne)
* **❌ L'erreur :** Penser en coordonnées cartésiennes (X, Y) pour lire une matrice : `matrice.at<double>(X, Y)`.
* **🚨 Pourquoi c'est faux :** L'informatique stocke les tableaux 2D en Ligne (Row) puis Colonne (Col). 
  - La ligne = l'axe vertical (Y). 
  - La colonne = l'axe horizontal (X).
* **✅ La correction :** `matrice.at<double>(Y, X)` ou plus rigoureusement `matrice.at<double>(ligne, colonne)`.
* **🧠 Règle d'or : "Row-Major".** En OpenCV, Numpy ou Eigen, on parcourt toujours (Ligne, Colonne). 

---

## 6. Syntaxe C++ / OpenCV Diverses
* **Le Try/Catch sur la détection :** Pas besoin de `try/catch` pour chercher si une image contient un ArUco. Si rien n'est trouvé, OpenCV renvoie juste un tableau d'IDs vide (`ids.size() == 0`).
* **L'appel de fonction :** En C++, on ne met jamais les types (`const std::vector&`) quand on *appelle* une fonction, uniquement quand on la *déclare*.
* **Pointeurs vers le bridge :** On utilise `cv_bridge` pour faire la passerelle mémoire sécurisée entre `sensor_msgs` et `cv::Mat`.