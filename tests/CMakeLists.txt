cmake_minimum_required(VERSION 2.8)

include(test)

add_subdirectory(agge)
add_subdirectory(agge.text)
if (NOT MSVC_VERSION OR NOT (MSVC_VERSION LESS 1700))
	add_subdirectory(agge.async)
endif()
add_subdirectory(common/src)
