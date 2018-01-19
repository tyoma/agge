#include <agge/platform/bitmap.h>

#include <CoreGraphics/CoreGraphics.h>
#include <new>

namespace agge
{
	namespace platform
	{
		namespace
		{
			const void *get_buffer(void *data)
			{	return data;	}
			
			template <typename T>
			void swap(T &lhs, T &rhs)
			{	T t = lhs; lhs = rhs, rhs = t;	}
		}
		
		raw_bitmap::~raw_bitmap()
		{
			if (_native)
				::CGImageRelease(_native);
			delete []_memory;
		}
		
		void raw_bitmap::resize(count_t width, count_t height)
		{
			count_t stride = calculate_stride(width + _extra_pixels);
			count_t size = stride * height;
			uint8_t *memory = size > _allocated_width ? new uint8_t[size] : _memory;
			size_t component_bits = _bpp == bpp32 ? 8 : _bpp == bpp16 ? 5 : 8;
			int format = _bpp == bpp32 ? kCGImageAlphaNoneSkipFirst : kCGImageAlphaNone;
			
			CGDataProviderDirectCallbacks cb = { 0, &get_buffer, 0, 0, 0 };
			CGColorSpaceRef cs = _bpp == bpp8 ? ::CGColorSpaceCreateDeviceGray() : ::CGColorSpaceCreateDeviceRGB();
			CGDataProviderRef provider = ::CGDataProviderCreateDirect(memory, size, &cb);
			CGImageRef image = ::CGImageCreate(width, height, component_bits, _bpp, stride, cs, format, provider, NULL,
				FALSE, kCGRenderingIntentDefault);
			::CGDataProviderRelease(provider);
			::CGColorSpaceRelease(cs);
			
			if (image)
			{
				if (_native)
					::CGImageRelease(_native);
				_allocated_width = memory != _memory ? size : _allocated_width;
				_native = image;
				_width = width;
				_height = height;
				_stride = stride;
				swap(_memory, memory);
			}
			if (memory != _memory)
				delete []memory;
			if (!image)
				throw std::bad_alloc();			
		}
		
		void raw_bitmap::blit(CGContextRef context, int x, int y, count_t width, count_t height) const
		{
			CGRect destination = { { CGFloat(x), CGFloat(y) }, { CGFloat(_width), CGFloat(_height) } };
			CGRect clip = { { CGFloat(x), CGFloat(y) }, { CGFloat(width), CGFloat(height) } };
			
			::CGContextSaveGState(context);
			::CGContextClipToRect(context, clip);
			::CGContextDrawImage(context, destination, _native);
			::CGContextRestoreGState(context);
		}
	}
}
