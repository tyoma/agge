#pragma once

#include <agge/config.h>
#include <agge/pixel.h>

#if defined(AGGE_PLATFORM_APPLE)
	struct CGImage;
	typedef struct CGImage *image_handle;
	struct CGContext;
	typedef struct CGContext *context_handle;
#elif defined(AGGE_PLATFORM_WINDOWS)
	struct HBITMAP__;
	typedef struct HBITMAP__ *image_handle;
	struct HDC__;
	typedef struct HDC__ *context_handle;
#else
	typedef void *image_handle;
	typedef void *context_handle;
#endif

namespace agge
{
	namespace platform
	{
		class raw_bitmap : noncopyable
		{
		public: // General
			raw_bitmap(count_t width, count_t height, bits_per_pixel bpp, count_t row_extra_bytes = 0);
			~raw_bitmap();

			void resize(count_t width, count_t height);

			bits_per_pixel bpp() const;
			count_t width() const;
			count_t height() const;
			count_t stride() const;

			void *row_ptr(count_t y);
			const void *row_ptr(count_t y) const;

		public:
			image_handle native() const;
			void blit(context_handle context, int x, int y, count_t width, count_t height) const;

		private:
			count_t calculate_stride(count_t width) const;

		private:
			uint8_t *_memory;
			count_t _stride;
			count_t _width, _allocated_width, _height, _allocated_height;
			const bits_per_pixel _bpp;
			image_handle _native;
			const count_t _extra_pixels;
		};



		inline raw_bitmap::raw_bitmap(count_t width, count_t height, bits_per_pixel bpp, count_t row_extra_bytes)
			: _memory(0), _allocated_width(0), _allocated_height(0), _bpp(bpp), _native(0),
				_extra_pixels((row_extra_bytes + bpp / 8 - 1) / (bpp / 8))
		{	resize(width, height);	}

		inline bits_per_pixel raw_bitmap::bpp() const
		{	return _bpp;	}

		inline count_t raw_bitmap::width() const
		{	return _width;	}

		inline count_t raw_bitmap::height() const
		{	return _height;	}

		inline count_t raw_bitmap::stride() const
		{	return _stride;	}

		inline void *raw_bitmap::row_ptr(count_t y)
		{	return _memory + y * _stride;	}

		inline const void *raw_bitmap::row_ptr(count_t y) const
		{	return _memory + y * _stride;	}
		
		inline image_handle raw_bitmap::native() const
		{	return _native;	}

		inline count_t raw_bitmap::calculate_stride(count_t width) const
		{	return (width * (_bpp / 8) + 3) & ~3;	}
	}
}
