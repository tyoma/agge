set(AGGE_SOURCES
	blenders_intel.cpp
	curves.cpp
	dash.cpp
	figures.cpp
	math.cpp
	parallel.cpp
	stroke.cpp
	stroke_features.cpp
	vector_rasterizer.cpp
)

if (WIN32)
	set(AGGE_SOURCES ${AGGE_SOURCES} platform/win32/bitmap.cpp)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++03")

add_library(agge STATIC ${AGGE_SOURCES})
