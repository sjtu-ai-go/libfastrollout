
cmake_minimum_required(VERSION 3.1)
project(libfastrollout)

option(libfastrollout_build_tests "Build libfastrollout's own tests" OFF)

set(CMAKE_CXX_STANDARD 11)

################################
# Find thread
################################
find_package(Threads REQUIRED)

################################
# Find boost
################################
find_package(Boost 1.46 REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})

##################################
# libgoboard
##################################
if (NOT TARGET goboard)
    add_subdirectory(vendor/libgoboard)
    set(libgoboard_SOURCE_DIR vendor/libgoboard)
    set(libgoboard_LIBS goboard)
    include_directories(${libgoboard_INCLUDE_DIR})
endif()

##################################
# libfastrollout
##################################
include_directories(src/)
add_library(fastrollout STATIC src/fastrollout/fastrollout.cpp src/fastrollout/fastrollout.hpp src/fastrollout/fastrollout_cnn.hpp)
target_link_libraries(fastrollout ${libgoboard_LIBS} ${Boost_SYSTEM_LIBRARY})
set(libfastrollout_INCLUDE_DIR ${libfastrollout_SOURCE_DIR}/src ${libgoboard_INCLUDE_DIR} ${libgoboard_SOURCE_DIR}/src PARENT_SCOPE)

#################################
# tests
################################
if (libfastrollout_build_tests)
    enable_testing()
    ################################
    # GTest
    ################################
    ADD_SUBDIRECTORY (vendor/googletest/googletest)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
    ###############################
    # board-test
    ###############################
    add_executable(fastrollout-test src/fastrollout_test.cpp)
    target_link_libraries(fastrollout-test fastrollout gtest gtest_main)
    add_test(fastrollout_test fastrollout-test)
    ###############################
    # cnntest
    ###############################
    add_executable(fr-cnn-test src/fastrollout_cnn_test.cpp)
    target_link_libraries(fr-cnn-test fastrollout gtest gtest_main)
    add_test(fastrollout_cnn_test fr-cnn-test)
endif()
