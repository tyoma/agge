set(AGGE_TEST_SOURCES
	BitmapTests.cpp
	BlendersTests.cpp
	ClipperTests.cpp
	DashTests.cpp
	FillingRulesTests.cpp
	GenericBlendersTests.cpp
	main.cpp
	MiscTests.cpp
	ParallelExecutionTests.cpp
	PathTests.cpp
	PodVectorTests.cpp
	PreciseDeltaTests.cpp
	RasterizerTests.cpp
	RendererTests.cpp
	RendererParallelTests.cpp
	ScanlineAdapterTests.cpp
	SIMDBlendersTests.cpp
	StrokeFeaturesTests.cpp
	StrokeTests.cpp
	VectorRasterizerTests.cpp
)

if (WIN32)
	set(AGGE_TEST_SOURCES ${AGGE_TEST_SOURCES} platform/win32/mt.cpp platform/win32/RawBitmapTests.cpp platform/win32/surface.cpp)
else()
	set(AGGE_TEST_SOURCES ${AGGE_TEST_SOURCES} platform/linux/mt.cpp)
endif()

find_package(Threads)

add_executable(agge.tests ${AGGE_TEST_SOURCES})

target_link_libraries(agge.tests agge utee ${CMAKE_THREAD_LIBS_INIT})
