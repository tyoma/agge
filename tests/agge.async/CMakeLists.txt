cmake_minimum_required(VERSION 3.13)

add_compile_options("$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-std=c++11>")

set(AGGE_ASYNC_TEST_SOURCES
	WorkerTests.cpp
)

add_library(agge.async.tests SHARED ${AGGE_ASYNC_TEST_SOURCES})

target_link_libraries(agge.async.tests agge.async tests-common)
