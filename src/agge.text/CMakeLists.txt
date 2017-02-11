set(AGGE_TEXT_SOURCES
	font.cpp
	layout.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_library(agge.text STATIC ${AGGE_TEXT_SOURCES})
