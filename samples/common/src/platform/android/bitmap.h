#pragma once

#include <agge/pixel.h>
#include <agge/types.h>

typedef struct ANativeWindow ANativeWindow;

class android_native_surface : agge::noncopyable
{
public:
	typedef agge::pixel32 pixel;

public:
	android_native_surface(ANativeWindow &window);
	~android_native_surface();

	unsigned width() const;
	unsigned height() const;

	pixel *row_ptr(unsigned y);

private:
	ANativeWindow &_window;

	agge::count_t _width, _height, _stride;
	void *_buffer;
};



inline unsigned android_native_surface::width() const
{	return _width;	}

inline unsigned android_native_surface::height() const
{	return _height;	}

inline android_native_surface::pixel *android_native_surface::row_ptr(unsigned y)
{	return reinterpret_cast<pixel *>(static_cast<agge::uint8_t *>(_buffer) + y * _stride);	}
