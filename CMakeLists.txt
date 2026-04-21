cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(BasicImageSegmenter)

set(OpenCV_DIR "/mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-install/lib/cmake/opencv4")

find_package(OpenCV REQUIRED)
find_package(yaml-cpp REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS} ${YAML_CPP_INCLUDE_DIRS})

add_executable(basicseg main.cpp segmenter.cpp segmenterFactory.cpp featureExtractor.cpp)
target_link_libraries(basicseg PRIVATE ${OpenCV_LIBS} yaml-cpp)

message(STATUS "OpenCV_DIR: ${OpenCV_DIR}")
message(STATUS "Include Directory: ${OpenCV_INCLUDE_DIRS}")