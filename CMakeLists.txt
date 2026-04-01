cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(BasicImageSegmenter)

set(OpenCV_DIR "/mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-install/lib/cmake/opencv4")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(basicseg basicseg.cpp segmenter.cpp)
target_link_libraries(basicseg ${OpenCV_LIBS})

message(STATUS "OpenCV_DIR: ${OpenCV_DIR}")
message(STATUS "Include Directory: ${OpenCV_INCLUDE_DIRS}")