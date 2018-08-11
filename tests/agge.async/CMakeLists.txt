include_directories(${AGGE_SOURCE_DIR}/libraries/poly-queue)

set(AGGE_ASYNC_TEST_SOURCES
	CircularBufferTests.cpp
	PolyEntryTests.cpp
	QueueTests.cpp
	WorkerTests.cpp
)

find_package(Threads)

add_library(agge.async.tests SHARED ${AGGE_ASYNC_TEST_SOURCES})

target_link_libraries(agge.async.tests agge tests-common utee ${CMAKE_THREAD_LIBS_INIT})
