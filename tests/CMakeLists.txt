include_directories(${AGGE_SOURCE_DIR}/libraries/utee)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti")
endif()

add_subdirectory(agge)
add_subdirectory(agge.text)
add_subdirectory(agge.async)
add_subdirectory(common/src)
