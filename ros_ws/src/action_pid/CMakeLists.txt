cmake_minimum_required(VERSION 3.8)
project(action_pid)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 1. Recherche des paquets standards
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(geometry_msgs REQUIRED)

# 2. Recherche du générateur d'interfaces (INDISPENSABLE)
find_package(rosidl_default_generators REQUIRED)

# 3. Génération du fichier .action
rosidl_generate_interfaces(${PROJECT_NAME}
  "action/FollowTarget.action"
)

# 4. Configuration du Nœud Action Server
add_executable(target_follower_node src/target_follower_node.cpp.cpp)
ament_target_dependencies(target_follower_node rclcpp rclcpp_action tf2 tf2_ros geometry_msgs)
# Liaison avec l'action générée dans CE package
rosidl_target_interfaces(target_follower_node ${PROJECT_NAME} "rosidl_typesupport_cpp")

# 5. Configuration du Nœud Action Client
add_executable(action_client_example src/action_client_example.cpp)
ament_target_dependencies(action_client_example rclcpp rclcpp_action)
# Liaison avec l'action générée dans CE package
rosidl_target_interfaces(action_client_example ${PROJECT_NAME} "rosidl_typesupport_cpp")

# 6. Installation
install(TARGETS
  target_follower_node
  action_client_example
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()