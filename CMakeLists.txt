cmake_minimum_required(VERSION 2.8)

project(AGGE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_bin)

if (NOT TARGET utee)
	set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules")
endif()

include_directories(.)

add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)

set(GCC_CLANG_COMPILER_OPTIONS_CXX -std=c++11 -fPIC -Wall -fno-rtti)
if(NOT ANDROID_ABI OR ANDROID_ABI MATCHES "x86.*")
	set(GCC_CLANG_COMPILER_OPTIONS_CXX ${GCC_CLANG_COMPILER_OPTIONS_CXX} -msse2)
endif()	
add_compile_options("$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:${GCC_CLANG_COMPILER_OPTIONS_CXX}>")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")

add_subdirectory(src/agge)
add_subdirectory(src/agge.text)
add_subdirectory(tests)

if (NOT TARGET utee)
	add_subdirectory(libraries/utee/src)
endif()

set_property(TARGET agge APPEND PROPERTY COMPILE_OPTIONS $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-std=c++03>)
set_property(TARGET agge.tests APPEND PROPERTY COMPILE_OPTIONS $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-std=c++03>)
