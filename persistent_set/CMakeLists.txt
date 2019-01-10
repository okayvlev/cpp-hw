project(set_test)

cmake_minimum_required(VERSION 2.8)

include_directories(${set_test_SOURCE_DIR})

add_executable(set_test test.cpp gtest/gtest_main.cc gtest/gtest-all.cc)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -pedantic -O2")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fsanitize=address,undefined -D_GLIBCXX_DEBUG")
endif()

target_link_libraries(set_test -lpthread)
