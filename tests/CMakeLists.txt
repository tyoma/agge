include_directories(${AGGE_SOURCE_DIR}/libraries/utee)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -frtti")

add_subdirectory(agge)
add_subdirectory(agge.text)
