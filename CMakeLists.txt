project(bigint_testing)

SET(CMAKE_CXX_FLAGS  "-Wall -pedantic -std=c++11 -O2")

add_executable(bigint_testing test.cpp big_integer.cpp)
