cmake_minimum_required(VERSION 3.13)

add_compile_options("$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-std=c++11>")

set(AGGE_TEXT_TEST_SOURCES
	AnnotatedStringTests.cpp
	FontTests.cpp
	helpers_layout.cpp
	LayoutBuilderTests.cpp
	LayoutTests.cpp
	LineLimitTests.cpp
	mocks.cpp
	RichTextLayoutTests.cpp
	RichTextTests.cpp
	TextEngineRichTextTests.cpp
	TextEngineTests.cpp
)

add_library(agge.text.tests SHARED ${AGGE_TEXT_TEST_SOURCES})

target_link_libraries(agge.text.tests agge.text)
