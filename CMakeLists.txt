cmake_minimum_required(VERSION 3.10)
project(BasicImageSegmenter)

set(OpenCV_DIR "/mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-install/lib/cmake/opencv4")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(basicseg basicseg.cpp)
target_link_libraries(basicseg ${OpenCV_LIBS})

message(STATUS "OpenCV_DIR: ${OpenCV_DIR}")
message(STATUS "Include Directory: ${OpenCV_INCLUDE_DIRS}")