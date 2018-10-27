set(AGGE_SOURCES
	blenders_intel.cpp
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

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++03")
endif()

add_library(agge STATIC ${AGGE_SOURCES})
