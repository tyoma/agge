#pragma once

#include <agge/bitmap.h>
#include <agge/blenders.h>
#include <agge/blenders_simd.h>
#include <agge/config.h>

#if defined(AGGE_PLATFORM_ANDROID)
	#include "src/platform/android/bitmap.h"

	typedef android_native_surface platform_bitmap;
	typedef agge::order_rgba platform_pixel_order;

#elif defined(AGGE_PLATFORM_WINDOWS)
	#include <agge/platform/bitmap.h>

	typedef agge::bitmap<agge::pixel32, agge::platform::raw_bitmap> platform_bitmap;
	typedef agge::order_bgra platform_pixel_order;

#elif defined(AGGE_PLATFORM_APPLE)
	#include <agge/platform/bitmap.h>

	typedef agge::bitmap<agge::pixel32, agge::platform::raw_bitmap> platform_bitmap;
	typedef agge::order_argb platform_pixel_order;

#endif

typedef agge::blender_solid_color<agge::simd::blender_solid_color, platform_pixel_order> platform_blender_solid_color;

struct services;

struct application : agge::noncopyable
{
	struct timings
	{
		double clearing;
		double stroking;
		double rasterization;
		double rendition;
		double blitting;
	};

	virtual ~application();

	virtual void draw(platform_bitmap &surface, timings &timings_) = 0;
	virtual void resize(int width, int height);
};



extern application *agge_create_application(services &/*s*/);
