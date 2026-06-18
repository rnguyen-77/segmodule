cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(BasicImageSegmenter)

# Bundled OpenCV install lives inside the source tree. Override at configure time
# with -DOpenCV_DIR=<path> if yours is elsewhere.
set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/opencv-install/lib/cmake/opencv4"
    CACHE PATH "Directory containing OpenCVConfig.cmake")

find_package(OpenCV REQUIRED)
find_package(yaml-cpp REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS} ${YAML_CPP_INCLUDE_DIRS})
# boost (multi_array is header-only in /usr/include; iostreams gzip needs the lib).
# Linked by name to avoid a broken FindBoost imported target on this box.

# Path to the bagLoader module (bag.h / bag.cpp / file_io.h). Not in this repo;
# set it at configure time with -DBAGLOADER_DIR=<path> or the BAGLOADER_DIR env var.
# basicseg builds only when it is provided; the test target does not need it.
set(BAGLOADER_DIR "$ENV{BAGLOADER_DIR}"
    CACHE PATH "Path to the bagLoader module providing bag.cpp / bag.h / file_io.h")

if(BAGLOADER_DIR)
    # basicseg: the real program. Loads a .mi.gz bag via the bagLoader, then segments.
    add_executable(basicseg
        main.cpp
        segmenter.cpp
        segmenterFactory.cpp
        seg_image.cpp
        bag_adapter.cpp
        ${BAGLOADER_DIR}/bag.cpp        # bagLoader's ALOG::Bag loader implementation
    )
    target_include_directories(basicseg PRIVATE ${BAGLOADER_DIR})
    target_link_libraries(basicseg PRIVATE ${OpenCV_LIBS} yaml-cpp boost_iostreams z)
else()
    message(WARNING
        "BAGLOADER_DIR not set; skipping the basicseg program. "
        "Pass -DBAGLOADER_DIR=<path> to build it. The segmenter_test target still builds.")
endif()

message(STATUS "Include Directory: ${OpenCV_INCLUDE_DIRS}")

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(segmenter_test
    tests/segmenter_test.cpp
    segmenter.cpp
    segmenterFactory.cpp
)
target_link_libraries(segmenter_test PRIVATE
    ${OpenCV_LIBS}
    yaml-cpp
    GTest::GTest
    GTest::Main
)

# Copy the test config fixture next to the test binary after every build, so the
# tests resolve it relative to the executable (see configPath() in segmenter_test.cpp)
# and pass no matter which directory the executable is launched from.
add_custom_command(TARGET segmenter_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/tests/test_config.yaml
        $<TARGET_FILE_DIR:segmenter_test>/test_config.yaml
    COMMENT "Staging test_config.yaml next to segmenter_test"
)

add_test(NAME segmenter_test COMMAND segmenter_test)