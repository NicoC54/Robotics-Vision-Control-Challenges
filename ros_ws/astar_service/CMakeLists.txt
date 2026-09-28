cmake_minimum_required(VERSION 3.8)
project(astar_service)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces (${PROJECT_NAME} "srv/GetPath.srv")



add_executable(client_node src/client.cpp)
ament_target_dependencies(
  client_node
  rclcpp
)
rosidl_target_interfaces(client_node ${PROJECT_NAME} "rosidl_typesupport_cpp")



add_executable(service_node src/service.cpp)
ament_target_dependencies(
  service_node
  rclcpp
)
rosidl_target_interfaces(service_node ${PROJECT_NAME} "rosidl_typesupport_cpp")

# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

install (TARGETS 
client_node
service_node
DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
