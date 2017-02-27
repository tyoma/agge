set(AGGE_TEXT_TEST_SOURCES
	FontTests.cpp
	LayoutTests.cpp
	main.cpp
	mocks.cpp
)

add_executable(agge.text.tests ${AGGE_TEXT_TEST_SOURCES})

target_link_libraries(agge.text.tests agge.text utee)
