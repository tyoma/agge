include_directories(${AGGE_SOURCE_DIR}/misc/libraries/utee)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(agge.tests
	BlenderTests.cpp
	ClipperTests.cpp
	DashTests.cpp
	helpers.cpp
	main.cpp
	MiscTests.cpp
	PathTests.cpp
	PodVectorTests.cpp
	PreciseDeltaTests.cpp
	RasterizerTests.cpp
	RendererTests.cpp
	ScanlineAdapterTests.cpp
	StrokeFeaturesTests.cpp
	StrokeTests.cpp
	VectorRasterizerTests.cpp
)
target_link_libraries(agge.tests agge utee)
