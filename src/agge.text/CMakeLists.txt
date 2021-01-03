cmake_minimum_required(VERSION 2.8)

set(AGGE_TEXT_SOURCES
	font.cpp
	layout.cpp
	richtext.cpp
	text_engine_base.cpp
	types.cpp
)

add_library(agge.text STATIC ${AGGE_TEXT_SOURCES})

