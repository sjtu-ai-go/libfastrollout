
cmake_minimum_required(VERSION 3.1)
project(libfastrollout)

option(libfastrollout_build_tests "Build libfastrollout's own tests" OFF)

set(CMAKE_CXX_STANDARD 11)

################################
# Find thread
################################
find_package(Threads REQUIRED)

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
add_library(fastrollout STATIC src/fastrollout/fastrollout.cpp)
target_link_libraries(fastrollout ${libgoboard_LIBS})
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
endif()
