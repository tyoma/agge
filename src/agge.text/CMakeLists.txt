cmake_minimum_required(VERSION 3.13)

add_compile_options("$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-std=c++11>")

set(AGGE_TEXT_SOURCES
	font.cpp
	layout_builder.cpp
	richtext.cpp
	text_engine_base.cpp
	types.cpp
)

add_library(agge.text STATIC ${AGGE_TEXT_SOURCES})
target_link_libraries(agge.text PUBLIC agge PUBLIC utfia)
