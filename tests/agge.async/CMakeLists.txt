cmake_minimum_required(VERSION 2.8)

set(AGGE_ASYNC_TEST_SOURCES
	CircularBufferTests.cpp
	PolyEntryTests.cpp
	QueueTests.cpp
	WorkerTests.cpp
)

add_library(agge.async.tests SHARED ${AGGE_ASYNC_TEST_SOURCES})

target_link_libraries(agge.async.tests agge tests-common)
