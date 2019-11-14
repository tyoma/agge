cmake_minimum_required(VERSION 2.8)

set(AGGE_TEST_SOURCES
	BitmapTests.cpp
	BlendersTests.cpp
	ClipperTests.cpp
	CurvesTests.cpp
	DashTests.cpp
	FiguresTests.cpp
	FillingRulesTests.cpp
	GenericBlendersTests.cpp
	MiscTests.cpp
	ParallelExecutionTests.cpp
	PathJoiningTests.cpp
	PathTests.cpp
	PodVectorTests.cpp
	PreciseDeltaTests.cpp
	RasterizerTests.cpp
	RendererTests.cpp
	RendererParallelTests.cpp
	ScanlineAdapterTests.cpp
	StrokeFeaturesTests.cpp
	StrokeTests.cpp
	VectorRasterizerTests.cpp
)

if (WIN32)
	set(AGGE_TEST_SOURCES ${AGGE_TEST_SOURCES} platform/win32/RawBitmapTests.cpp platform/win32/surface.cpp)
else()
endif()

if(NOT ANDROID_ABI OR ANDROID_ABI MATCHES "x86.*")
	set(AGGE_TEST_SOURCES ${AGGE_TEST_SOURCES} SIMDBlendersTests.cpp)
endif()	

add_library(agge.tests SHARED ${AGGE_TEST_SOURCES})

target_link_libraries(agge.tests agge tests-common)
