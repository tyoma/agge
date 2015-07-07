#pragma once

#include "basics.h"

#include <agge/types.h>

struct HBITMAP__;
typedef struct HBITMAP__ *HBITMAP;

struct HDC__;
typedef struct HDC__ *HDC;

namespace aggx
{
	class bitmap
	{
	public:
		typedef agge::pixel32 pixel;

	public:
		bitmap(unsigned width, unsigned height);
		~bitmap();

		void blit(HDC hdc, int x, int y, int w, int h);
		pixel *access(unsigned x, unsigned y);

		unsigned width() const;
		unsigned height() const;

	private:
		bitmap(const bitmap &other);
		const bitmap &operator =(const bitmap &other);

	private:
		const unsigned _width, _height, _stride;
		HBITMAP _handle;
		void *_memory;
	};



	inline bitmap::pixel *bitmap::access(unsigned x, unsigned y)
	{	return reinterpret_cast<pixel *>(_memory) + y * _stride + x;	}

	inline unsigned bitmap::width() const
	{	return _width;	}

	inline unsigned bitmap::height() const
	{	return _height;	}
}
