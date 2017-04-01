#pragma once

#include <agge/bitmap.h>
#include <agge/platform/win32/bitmap.h>

typedef agge::bitmap<agge::pixel32, agge::platform::raw_bitmap> platform_bitmap;

struct shell
{
	struct application;

	virtual void present(application &app) = 0;
};

struct shell::application
{
	struct timings
	{
		double clearing;
		double stroking;
		double rasterization;
		double rendition;
		double blitting;
	};

	virtual void draw(platform_bitmap &surface, timings &timings_) = 0;
	virtual void resize(int width, int height);
};

// AGGE sample application entry point.
void agge_sample_main(shell &sh);



inline void shell::application::resize(int /*width*/, int /*height*/)
{	}
