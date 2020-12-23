#include <agge/platform/bitmap.h>

#include <CoreGraphics/CoreGraphics.h>
#include <new>

namespace agge
{
	namespace platform
	{
		namespace
		{
			template <typename T>
			void swap(T &lhs, T &rhs)
			{	T t = lhs; lhs = rhs, rhs = t;	}
		}
		
		raw_bitmap::~raw_bitmap()
		{
			delete []_memory;
		}
		
		void raw_bitmap::resize(count_t width, count_t height)
		{
			count_t stride = calculate_stride(width + _extra_pixels);
			count_t size = stride * height;
			
			if (size > _allocated_width /*size, actually*/)
			{
				uint8_t *memory = new uint8_t[size];
				
				_allocated_width = size;
				swap(_memory, memory);
				delete []memory;
			}
			_width = width;
			_height = height;
			_stride = stride;
		}
		
		void raw_bitmap::blit(CGContextRef context, int x, int y, count_t width, count_t height) const
		{
			count_t stride = calculate_stride(_width + _extra_pixels);
			count_t size = stride * height;
			size_t component_bits = _bpp == bpp32 ? 8 : _bpp == bpp16 ? 5 : 8;
			int format = _bpp == bpp32 ? kCGImageAlphaNoneSkipFirst : kCGImageAlphaNone;
			
			CGColorSpaceRef cs = _bpp == bpp8 ? ::CGColorSpaceCreateDeviceGray() : ::CGColorSpaceCreateDeviceRGB();
			CGDataProviderRef provider = ::CGDataProviderCreateWithData(0, _memory, size, 0);
			CGImageRef image = ::CGImageCreate(width, height, component_bits, _bpp, stride, cs, format, provider, NULL,
				FALSE, kCGRenderingIntentDefault);
			::CGDataProviderRelease(provider);
			::CGColorSpaceRelease(cs);

		
			CGRect destination = { { CGFloat(x), CGFloat(y) }, { CGFloat(width), CGFloat(height) } };
			
			::CGContextDrawImage(context, destination, image);
			::CGImageRelease(image);
		}
	}
}
