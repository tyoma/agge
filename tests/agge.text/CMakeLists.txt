set(AGGE_TEXT_TEST_SOURCES
	FontTests.cpp
	LayoutTests.cpp
	mocks.cpp
	TextEngineTests.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_library(agge.text.tests SHARED ${AGGE_TEXT_TEST_SOURCES})

target_link_libraries(agge.text.tests agge.text utee)
