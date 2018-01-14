set(AGGE_TEXT_SOURCES
	font.cpp
	layout.cpp
	text_engine.cpp
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -frtti")
endif()

add_library(agge.text STATIC ${AGGE_TEXT_SOURCES})
