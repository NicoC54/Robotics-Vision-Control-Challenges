cmake_minimum_required(VERSION 3.8)
project(perception_pipeline) # Doit correspondre EXACTEMENT au package.xml

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED) 

# L'exécutable garde le nom spécifique
add_executable(perception_pipeline_multithread src/perception_pipeline_multithread.cpp)

target_include_directories(perception_pipeline_multithread PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(perception_pipeline_multithread PUBLIC 
  ${OpenCV_LIBS}
  Threads::Threads
)

install(TARGETS perception_pipeline_multithread
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()