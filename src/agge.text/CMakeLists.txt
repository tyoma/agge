cmake_minimum_required(VERSION 2.8)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -frtti")
endif()

set(AGGE_TEXT_SOURCES
	font.cpp
	layout.cpp
	text_engine.cpp
)

add_library(agge.text STATIC ${AGGE_TEXT_SOURCES})
