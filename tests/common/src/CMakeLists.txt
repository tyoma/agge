set(TESTS_COMMON_SOURCES
)

if (WIN32)
	set(TESTS_COMMON_SOURCES ${TESTS_COMMON_SOURCES} platform/win32/mt.cpp)
else()
	set(TESTS_COMMON_SOURCES ${TESTS_COMMON_SOURCES} platform/linux/mt.cpp)
endif()

find_package(Threads)

add_library(tests-common STATIC ${TESTS_COMMON_SOURCES})
