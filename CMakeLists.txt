project(AGGE)

include_directories(${AGGE_SOURCE_DIR})

add_definitions(-D_SCL_SECURE_NO_WARNINGS)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_subdirectory(src/agge)
add_subdirectory(tests/agge)
add_subdirectory(misc/libraries/utee/src)
