set(AGGE_SOURCES
 blenders_intel.cpp
 dash.cpp
 math.cpp
 parallel.cpp
 stroke.cpp
 stroke_features.cpp
 vector_rasterizer.cpp
)

if (WIN32)
    set(AGGE_SOURCES ${AGGE_SOURCES} platform/win32/bitmap.cpp)
endif()

find_package(Threads)

add_library(agge STATIC ${AGGE_SOURCES})

target_link_libraries (agge ${CMAKE_THREAD_LIBS_INIT})
