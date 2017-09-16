project(bigint_testing)

cmake_minimum_required(VERSION 3.5)

include_directories(${bigint_testing_SOURCE_DIR})

#SET(CMAKE_CXX_FLAGS  "-Wall -pedantic -std=c++11 -O2")
SET(CMAKE_CXX_FLAGS  "-Wall -pedantic -std=c++11 -g -fsanitize=address,undefined -D_GLIBCXX_DEBUG")

add_executable(bigint_testing test.cpp big_integer.cpp)
#add_executable(bigint_testing big_integer_testing.cpp big_integer.cpp gtest/gtest_main.cc gtest/gtest-all.cc)


target_link_libraries(bigint_testing -lpthread)
