cmake_minimum_required(VERSION 2.8)

project(AGGE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/modules)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_bin)
set(UTEE_NO_TESTS 1)

include_directories(.)
include_directories(./libraries/polyq)
if (MSVC)
	if (MSVC_VERSION LESS_EQUAL 1700)
		include_directories(./compat/msvc10-)
	endif()
endif()

add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)

set(GCC_CLANG_COMPILER_OPTIONS_CXX -std=c++11 -fPIC -Wall -fno-rtti)
if(NOT ANDROID_ABI OR ANDROID_ABI MATCHES "x86.*")
	set(GCC_CLANG_COMPILER_OPTIONS_CXX ${GCC_CLANG_COMPILER_OPTIONS_CXX} -msse2)
endif()	

add_compile_options(
	"$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:${GCC_CLANG_COMPILER_OPTIONS_CXX}>"
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:/Ox>>"
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:/Ob2>>"
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<EQUAL:4,${CMAKE_SIZEOF_VOID_P}>:/arch:SSE2>>"
)

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")

add_subdirectory(src/agge)
add_subdirectory(src/agge.text)
if (NOT AGGE_NO_TESTS)
	if (NOT TARGET utee)
		add_subdirectory(libraries/utee)
	endif()
	add_subdirectory(samples)
	add_subdirectory(tests)
	set_property(TARGET agge.tests APPEND PROPERTY COMPILE_OPTIONS $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-std=c++03>)
endif()

set_property(TARGET agge APPEND PROPERTY COMPILE_OPTIONS $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-std=c++03>)
