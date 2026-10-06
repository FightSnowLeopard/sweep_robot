cmake_minimum_required(VERSION 3.8)

project(diffdrive_arduino)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(serial REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(controller_manager REQUIRED)
find_package(rclcpp REQUIRED)
find_package(pluginlib REQUIRED)

add_library(diffdrive_arduino SHARED
  src/diffdrive_arduino.cpp
  src/wheel.cpp
  src/arduino_comms.cpp
)

target_include_directories(diffdrive_arduino
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(diffdrive_arduino
  hardware_interface
  controller_manager
  serial
  rclcpp
  pluginlib
)

pluginlib_export_plugin_description_file(
  hardware_interface
  robot_hardware.xml
)

install(
  TARGETS diffdrive_arduino
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  FILES robot_hardware.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_export_include_directories(include)

ament_export_libraries(
  diffdrive_arduino
)

ament_export_dependencies(
  hardware_interface
  controller_manager
  serial
  rclcpp
  pluginlib
)

ament_package()