project(huffman_testing)

SET(CMAKE_CXX_FLAGS  "-Wall -O2")

add_executable(huffman_testing test.cpp huffman.cpp)
