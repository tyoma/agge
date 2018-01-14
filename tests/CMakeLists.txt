include_directories(${AGGE_SOURCE_DIR}/libraries/utee)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -frtti")
endif()

add_subdirectory(agge)
add_subdirectory(agge.text)
