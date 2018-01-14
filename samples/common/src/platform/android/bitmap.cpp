#include "bitmap.h"

#include <android/native_window.h>

android_native_surface::android_native_surface(ANativeWindow &window)
	: _window(window)
{
	ANativeWindow_Buffer descriptor = { };
	
	if (ANativeWindow_lock(&_window, &descriptor, 0) < 0)
		throw 0;

	switch (descriptor.format)
	{
	case WINDOW_FORMAT_RGBA_8888:
	case WINDOW_FORMAT_RGBX_8888:
		break;

	default:
		// Unsupported format: unlock window and throw exception...
		ANativeWindow_unlockAndPost(&_window);
		throw 0;
	}

	_width = descriptor.width;
	_height = descriptor.height;
	_stride = descriptor.stride * sizeof(pixel);
	_buffer = descriptor.bits;
}

android_native_surface::~android_native_surface()
{
	ANativeWindow_unlockAndPost(&_window);
}
