#pragma once

#include <agge/bitmap.h>

#if defined(_M_X64) || defined(_M_IX86) || defined(__x86_64) || defined(__i386)
	#include <agge/blenders_simd.h>

	typedef agge::simd::blender_solid_color platform_blender_solid_color;

#elif defined(__arm__)
	#include <misc/experiments/common/blenders.h>

	typedef common::blender_solid_color platform_blender_solid_color;

#endif

#if defined(__ANDROID__)
	#include "src/platform/android/bitmap.h"

	typedef android_native_surface platform_bitmap;

#elif defined(_WIN32)
	#include <agge/platform/win32/bitmap.h>

	typedef agge::bitmap<agge::pixel32, agge::platform::raw_bitmap> platform_bitmap;

#else
#endif

struct application
{
	struct timings
	{
		double clearing;
		double stroking;
		double rasterization;
		double rendition;
		double blitting;
	};

	application();

	virtual void draw(platform_bitmap &surface, timings &timings_) = 0;
	virtual void resize(int width, int height);
};



extern application *agge_create_application();
