project(bigint_testing)

#SET(CMAKE_CXX_FLAGS  "-Wall -pedantic -std=c++11 -O2")
SET(CMAKE_CXX_FLAGS  "-Wall -pedantic -std=c++11 -g -fsanitize=address,undefined -D_GLIBCXX_DEBUG")

add_executable(bigint_testing test.cpp big_integer.cpp)
