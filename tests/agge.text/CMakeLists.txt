include_directories(${AGGE_SOURCE_DIR}/misc/libraries/utee)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(AGGE_TEXT_TEST_SOURCES
	main.cpp
)

add_executable(agge.text.tests ${AGGE_TEXT_TEST_SOURCES})

target_link_libraries(agge.text.tests agge.text utee)
