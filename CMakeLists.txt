project(variant_test)

cmake_minimum_required(VERSION 2.8)

include_directories(${variant_test_SOURCE_DIR})

add_executable(variant_test
                gtest/gtest_main.cc
                gtest/gtest-all.cc
                gtest/gtest.h
                unit_test.cpp
                #test.cpp
                )

set(CMAKE_C_COMPILER /usr/bin/gcc-7)
set(CMAKE_CXX_COMPILER /usr/bin/g++-7)

include_directories(/usr/include/c++/7/)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -fmax-errors=10")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fsanitize=address,undefined -D_GLIBCXX_DEBUG")
endif()

target_link_libraries(variant_test pthread)
