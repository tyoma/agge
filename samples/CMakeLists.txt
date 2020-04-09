cmake_minimum_required(VERSION 2.8)

add_subdirectory(common/src)
link_libraries(samples.common)

if (WIN32)
	add_custom_command(OUTPUT arial-1000.fnt COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/text/assets/arial-1000.fnt "$<TARGET_FILE_DIR:text>")
	
	add_executable(balls balls/balls.cpp balls/balls_data.cpp)
	add_executable(balls-async balls-async/balls-async.cpp balls-async/balls_data.cpp)
	add_executable(figures figures/figures.cpp)
	add_executable(lines lines/lines.cpp)
	add_executable(piechart piechart/piechart.cpp)
	add_executable(rasterizer rasterizer/rasterizer.cpp)
	add_executable(sandbox sandbox/sandbox.cpp)
	add_executable(text text/text.cpp arial-1000.fnt)
	add_executable(text-gdi text-gdi/text-gdi.cpp)
endif()
