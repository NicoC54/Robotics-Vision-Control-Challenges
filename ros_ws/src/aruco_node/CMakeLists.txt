cmake_minimum_required(VERSION 3.8)
project(aruco_node)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 1. Recherche des paquets ROS 2 nécessaires
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)

# 2. Recherche de la librairie externe OpenCV
find_package(OpenCV REQUIRED)

# 3. Création de l'exécutable (ton noeud)
add_executable(pose_estimator_node src/aruco_node.cpp)

# 4. Ajout des dossiers d'en-tête (headers) d'OpenCV
target_include_directories(pose_estimator_node PRIVATE ${OpenCV_INCLUDE_DIRS})

# 5. Liaison des dépendances ROS 2 (la magie de ament)
ament_target_dependencies(pose_estimator_node
  rclcpp
  sensor_msgs
  geometry_msgs
  cv_bridge
  tf2
  tf2_ros
)

# 6. Liaison des librairies OpenCV (méthode C++ classique)
target_link_libraries(pose_estimator_node ${OpenCV_LIBS})

# 7. Règle d'installation pour que 'ros2 run' puisse trouver le noeud
install(TARGETS
  pose_estimator_node
  DESTINATION lib/${PROJECT_NAME}
)

# Macro obligatoire à la fin de chaque CMakeLists ROS 2
ament_package()