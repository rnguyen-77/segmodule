# CMake generated Testfile for 
# Source directory: /mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-4.x/modules/highgui
# Build directory: /mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-4.x/build/modules/highgui
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(opencv_test_highgui "/mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-4.x/build/bin/opencv_test_highgui" "--gtest_output=xml:opencv_test_highgui.xml")
set_tests_properties(opencv_test_highgui PROPERTIES  LABELS "Main;opencv_highgui;Accuracy" WORKING_DIRECTORY "/mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-4.x/build/test-reports/accuracy" _BACKTRACE_TRIPLES "/mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-4.x/cmake/OpenCVUtils.cmake;1799;add_test;/mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-4.x/cmake/OpenCVModule.cmake;1365;ocv_add_test_from_target;/mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-4.x/modules/highgui/CMakeLists.txt;311;ocv_add_accuracy_tests;/mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-4.x/modules/highgui/CMakeLists.txt;0;")
