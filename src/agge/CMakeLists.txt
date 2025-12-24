cmake_minimum_required(VERSION 3.13)

add_compile_options($<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-std=c++03>)

set(AGGE_SOURCES
	color.cpp
	curves.cpp
	dash.cpp
	figures.cpp
	hybrid_event.cpp
	math.cpp
	parallel.cpp
	stroke.cpp
	stroke_features.cpp
	vector_rasterizer.cpp
)

if (AGGE_ARCHITECTURE STREQUAL "intel")
	set(AGGE_SOURCES ${AGGE_SOURCES}
		blenders_intel.cpp
	)
endif()

if (WIN32)
	set(AGGE_SOURCES ${AGGE_SOURCES}
		platform/win32/bitmap.cpp
		platform/win32/intrinsic.cpp
		platform/win32/thread.cpp
	)
else()
	set(AGGE_SOURCES ${AGGE_SOURCES}
		platform/unix/intrinsic.cpp
		platform/unix/thread.cpp
	)
endif()

if (APPLE)
	set(AGGE_SOURCES ${AGGE_SOURCES}
		platform/apple/bitmap.cpp
		platform/apple/semaphore.cpp
	)
elseif(WIN32)
	set(AGGE_SOURCES ${AGGE_SOURCES}
		platform/win32/semaphore.cpp
	)
else()
	set(AGGE_SOURCES ${AGGE_SOURCES}
		platform/linux/semaphore.cpp
	)
endif()

add_library(agge STATIC ${AGGE_SOURCES})

target_include_directories(agge PUBLIC ../..)
if (APPLE)
	target_link_libraries(agge PRIVATE "-framework Cocoa")
endif()

