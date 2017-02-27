cmake_minimum_required(VERSION 2.8)

project(AGGE)

include_directories(${AGGE_SOURCE_DIR})

add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)

add_subdirectory(src/agge)
add_subdirectory(src/agge.text)
add_subdirectory(tests)
add_subdirectory(libraries/utee/src)
