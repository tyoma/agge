include_directories(..)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_library(utee STATIC assert.cpp registry.cpp)
