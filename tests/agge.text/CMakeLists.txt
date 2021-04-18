cmake_minimum_required(VERSION 2.8)

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
	UTF8Tests.cpp
)

add_library(agge.text.tests SHARED ${AGGE_TEXT_TEST_SOURCES})

target_link_libraries(agge.text.tests agge.text agge)
