# CMake generated Testfile for 
# Source directory: /mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-4.x/modules/ml
# Build directory: /mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-4.x/build/modules/ml
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(opencv_test_ml "/mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-4.x/build/bin/opencv_test_ml" "--gtest_output=xml:opencv_test_ml.xml")
set_tests_properties(opencv_test_ml PROPERTIES  LABELS "Main;opencv_ml;Accuracy" WORKING_DIRECTORY "/mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-4.x/build/test-reports/accuracy" _BACKTRACE_TRIPLES "/mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-4.x/cmake/OpenCVUtils.cmake;1799;add_test;/mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-4.x/cmake/OpenCVModule.cmake;1365;ocv_add_test_from_target;/mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-4.x/cmake/OpenCVModule.cmake;1123;ocv_add_accuracy_tests;/mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-4.x/modules/ml/CMakeLists.txt;2;ocv_define_module;/mnt/netapp/SECURITY/users/rnguyen/segModule/opencv-4.x/modules/ml/CMakeLists.txt;0;")
