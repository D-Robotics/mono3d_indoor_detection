# Copyright (c) 2024，D-Robotics.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.5)
project(mono3d_indoor_detection)
#include(../../../../robot_dev_config/aarch64_toolchainfile.cmake)
#set(CMAKE_PREFIX_PATH "../../../../install/")
#set(CMAKE_CXX_FLAGS " -g ")
# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(ai_msgs REQUIRED)
find_package(dnn_node REQUIRED)
find_package(OpenCV 3.4.5 REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(hbm_img_msgs REQUIRED)

include_directories(include
  ${PROJECT_SOURCE_DIR}
)

# x3|x5|x86
set(PREFIX_PATH x3)
set(SYS_ROOT ${CMAKE_SYSROOT})

if(PLATFORM_X3)
  message("build platform X3")
  set(SYS_ROOT ${CMAKE_SYSROOT})
  set(PREFIX_PATH x3)
elseif(PLATFORM_Rdkultra)
  message("build platform Rdkultra")
elseif(PLATFORM_X5)
  message("build platform X5")
  set(PREFIX_PATH x5)
elseif(PLATFORM_X86)
  message("build platform X86")
  message("THIRD_PARTY is " ${THIRD_PARTY})
  add_definitions(-DPLATFORM_X86)
  set(SYS_ROOT ${THIRD_PARTY})
  link_directories(${THIRD_PARTY}/usr/lib)
  set(PREFIX_PATH x3)
else()
  message("invalid platform, build platform X3 default")
  set(SYS_ROOT ${CMAKE_SYSROOT})
endif()


message("CMAKE_SYSROOT is " ${CMAKE_SYSROOT})
include_directories(
  ${SYS_ROOT}/usr/include/
)
link_directories(
  ${SYS_ROOT}/usr/lib/hbbpu/
  ${SYS_ROOT}/usr/lib/hbmedia/
)

add_definitions(-DSHARED_MEM_ENABLED)

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/centernet_3d_detection_node.cpp
  src/centernet_3d_output_parser.cpp
)

add_executable(image_publisher
  src/image_publisher.cpp
)

ament_target_dependencies(
  ${PROJECT_NAME}
  rclcpp
  dnn_node
  ai_msgs
  hbm_img_msgs
  OpenCV
)

ament_target_dependencies(
  image_publisher
  rclcpp
  dnn_node
  OpenCV
  sensor_msgs
  hbm_img_msgs
)

# Install executables
install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(
  TARGETS image_publisher
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  ${PROJECT_SOURCE_DIR}/config/${PREFIX_PATH}/
  DESTINATION lib/${PROJECT_NAME}/config/
)

install(DIRECTORY
  ${PROJECT_SOURCE_DIR}/config/images
  DESTINATION lib/${PROJECT_NAME}/config/
)

install(FILES
  ${PROJECT_SOURCE_DIR}/config/centernet.yaml
  DESTINATION lib/${PROJECT_NAME}/config/
)

install(DIRECTORY
        launch
        DESTINATION share/${PROJECT_NAME})

ament_package()
