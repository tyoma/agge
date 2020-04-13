cmake_minimum_required(VERSION 2.8)

add_subdirectory(common/src)
link_libraries(samples.common)

if (WIN32 OR APPLE)

	set(MACOSX_RESOURCES)	

	if (APPLE)
		set(MACOSX_RESOURCES common/src/platform/apple/main.xib)
	endif()
	
	add_custom_command(OUTPUT arial-1000.fnt COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/text/assets/arial-1000.fnt "$<TARGET_FILE_DIR:text>")
	
	add_executable(balls MACOSX_BUNDLE balls/balls.cpp balls/balls_data.cpp ${MACOSX_RESOURCES})
	add_executable(balls-async balls-async/balls-async.cpp balls-async/balls_data.cpp ${MACOSX_RESOURCES})
	add_executable(figures figures/figures.cpp ${MACOSX_RESOURCES})
	add_executable(lines lines/lines.cpp ${MACOSX_RESOURCES})
	add_executable(piechart piechart/piechart.cpp ${MACOSX_RESOURCES})
	add_executable(rasterizer rasterizer/rasterizer.cpp ${MACOSX_RESOURCES})
	add_executable(sandbox sandbox/sandbox.cpp ${MACOSX_RESOURCES})
	add_executable(text text/text.cpp arial-1000.fnt ${MACOSX_RESOURCES})

	if (WIN32)
		add_executable(text-gdi text-gdi/text-gdi.cpp)
	endif()
	
	if (APPLE)
		set_target_properties(balls balls-async figures lines piechart rasterizer sandbox text PROPERTIES
			MACOSX_BUNDLE YES
			MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/common/src/platform/apple/info.plist
			RESOURCE ${MACOSX_RESOURCES}
			XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.agge.sample"
			XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
#			XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS ${CMAKE_CURRENT_SOURCE_DIR}/common/src/platform/apple/common.entitlements
			XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
			XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
  	)
	endif()
	
endif()
