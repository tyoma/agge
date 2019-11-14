cmake_minimum_required(VERSION 2.8)

set(AGGE_TEXT_TEST_SOURCES
	FontTests.cpp
	LayoutTests.cpp
	mocks.cpp
	TextEngineTests.cpp
)

set_property(SOURCE mocks.cpp PROPERTY COMPILE_FLAGS $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fno-rtti>)

add_library(agge.text.tests SHARED ${AGGE_TEXT_TEST_SOURCES})

target_link_libraries(agge.text.tests agge.text)
