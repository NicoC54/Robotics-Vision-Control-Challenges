# 🚀 LES 5 EXERCICES ULTIMES (Du plus simple au plus difficile)

## Niveau 1 : Le Pipeline de Perception (C++ Pur, Threading & Polymorphisme)

*Le classique pour tester votre code C++ et votre gestion mémoire bas niveau.*

* **Objectif :** Concevoir le cœur logiciel (hors ROS) d'une caméra. Le programme doit capturer des images en boucle continue. En parallèle, il doit appliquer une séquence de filtres d'image (ex : Flou Gaussien, puis Canny). Le système de filtrage doit être "Plug & Play" (on peut ajouter de nouveaux algorithmes sans toucher au cœur du code).
* **Le Piège d'Entretien (100% de chances de tomber) :** La `cv::Mat` est un pointeur déguisé (Shallow Copy). Si le thread de capture écrase l'image pendant que le thread de traitement la lit, le programme crash (Data Race). De plus, si vous bloquez le Mutex trop longtemps, vos FPS tombent à zéro.

---

## Niveau 2 : Le Planificateur A* (C++ STL, Big O & ROS 2 Services)

*Le test algorithmique parfait pour vérifier si vous savez optimiser votre code.*

* **Objectif :** Créer un nœud ROS 2 qui expose un Service `GetPath`. Le client envoie une grille 2D (occupancy grid), un point A et un point B. Le nœud calcule le chemin le plus court en évitant les obstacles en utilisant l'algorithme A* (A-Star), puis renvoie la liste des coordonnées.
* **Le Piège d'Entretien (100% de chances de tomber) :** L'usage de conteneurs inappropriés. Si vous utilisez un `std::vector` que vous triez (`std::sort`) à chaque itération pour trouver le nœud le moins coûteux, l'algorithme sera affreusement lent. Le recruteur attend une structure STL très spécifique avec une complexité d'extraction en $O(1)$ ou $O(\log n)$.

---

## Niveau 3 : Le Contrôleur de Poursuite (ROS 2 Actions, PID & TF2 Listener)

*La liaison entre l'architecture asynchrone ROS 2 et l'automatique.*

* **Objectif :** Développer un Action Server ROS 2 pour un robot mobile. L'objectif reçu est "Suivre l'objet X". Le nœud doit écouter en permanence l'arbre TF2 pour connaître la distance entre le `base_link` du robot et la cible. Il utilise un régulateur PID pour calculer la vitesse (`cmd_vel`) à envoyer aux roues. Il renvoie la distance restante en Feedback, et s'arrête si la cible est perdue (annulation).
* **Le Piège d'Entretien (100% de chances de tomber) :**
* *L'Integral Windup :* Si les roues du robot sont bloquées, l'erreur s'accumule dans le 'I' du PID. Quand on le débloque, il part à une vitesse folle. Comment le coder ?
* *L'Executor bloqué :* Si votre boucle PID utilise un bête `while(true)` ou `rclcpp::sleep()`, l'Action Server va bloquer le Callback Queue et le robot crashera.



---

## Niveau 4 : L'Estimateur de Pose 3D (OpenCV, TF2 Broadcaster, Quaternions)

*Le cœur de la vision robotique spatiale.*

* **Objectif :** Un nœud reçoit une image caméra. Vous détectez un marqueur ArUco (OpenCV), vous utilisez la matrice intrinsèque pour calculer sa pose 3D (Translation + Rotation) via `solvePnP`. Enfin, vous publiez cette pose dans le réseau ROS 2 pour que le reste du robot sache où se trouve la cible.
* **Le Piège d'Entretien (100% de chances de tomber) :** L'algorithme `solvePnP` renvoie un vecteur de rotation (Rodrigues). Pourquoi ne pas utiliser les Angles d'Euler (Roll/Pitch/Yaw) pour publier le TF2 ? C'est ici que vous devez expliquer le Gimbal Lock et appliquer mathématiquement vos Quaternions.

---

## Niveau 5 (Boss) : L'Estimateur d'État Zero-Copy (Kalman, QoS, Intra-process)

*L'exercice Senior. Si vous réussissez ça, vous êtes embauché.*

* **Objectif :** Concevoir deux Component Nodes tournant sur le même processus. Le premier lit la caméra (Publisher profil QoS `SensorData`). Le second s'y abonne, lit l'image en Zero-Copy (sans dupliquer la RAM), extrait la position (X, Y) d'un objet bruité, et passe ces mesures dans un Filtre de Kalman (KF) linéaire pour prédire la position réelle et ignorer le bruit.
* **Le Piège d'Entretien (100% de chances de tomber) :**
* *`toCvShare` vs `toCvCopy` :* Si vous modifiez l'image après un `toCvShare`, vous corrompez la mémoire de tous les autres nœuds.
* *Les conflits QoS :* Si le subscriber est configuré en Reliable et le publisher en Best Effort, les deux nœuds ne communiqueront jamais (silence radio sans message d'erreur).