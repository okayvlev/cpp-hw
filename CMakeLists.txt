project(huffman_testing)

SET(CMAKE_CXX_FLAGS  "-Wall -pedantic -std=c++11 -O2")

add_executable(huffman_testing test.cpp huffman_encoder.cpp)
