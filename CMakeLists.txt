cmake_minimum_required(VERSION 2.8)

project(AGGE)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${AGGE_SOURCE_DIR})

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
endif()

add_subdirectory(src/agge)
add_subdirectory(src/agge.text)
add_subdirectory(tests)
add_subdirectory(libraries/utee/src)
