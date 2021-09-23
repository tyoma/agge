cmake_minimum_required(VERSION 2.8)

include(test)

set(GCC_CLANG_COMPILER_OPTIONS_CXX ${GCC_CLANG_COMPILER_OPTIONS_CXX}
	-Wno-maybe-uninitialized
	-Wno-array-bounds
)

add_compile_options(
	"$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:${GCC_CLANG_COMPILER_OPTIONS_CXX}>"
)

find_package(Threads)

link_libraries(${CMAKE_THREAD_LIBS_INIT})

add_subdirectory(agge)
add_subdirectory(agge.text)
if (NOT MSVC_VERSION OR (MSVC_VERSION GREATER_EQUAL 1600))
	add_subdirectory(agge.async)
endif()
add_subdirectory(common/src)
