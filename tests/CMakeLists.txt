cmake_minimum_required(VERSION 3.13)

include_directories(BEFORE ..)

add_compile_options(
	"$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wno-array-bounds>"
	"$<$<CXX_COMPILER_ID:GNU>:-Wno-maybe-uninitialized>"
)

find_package(Threads)

link_libraries(${CMAKE_THREAD_LIBS_INIT})

add_subdirectory(agge)
add_subdirectory(agge.text)
if (NOT MSVC_VERSION OR (MSVC_VERSION GREATER_EQUAL 1600))
	add_subdirectory(agge.async)
endif()
add_subdirectory(common/src)
