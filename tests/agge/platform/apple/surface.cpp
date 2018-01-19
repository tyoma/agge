#include "surface.h"

#include <CoreGraphics/CoreGraphics.h>

using namespace std;

namespace agge
{
	namespace platform
	{
		namespace tests
		{
			gdi_surface::gdi_surface(count_t width_, count_t height_)
				: width(width_), height(height_), data(new uint8_t[4 * width * height])
			{
				CGColorSpaceRef cs = ::CGColorSpaceCreateDeviceRGB();
				
				memset(data, 0, 4 * width * height);
				_context = ::CGBitmapContextCreate(data, width, height, 8, 4 * width, cs, kCGImageAlphaNoneSkipLast);
				::CGColorSpaceRelease(cs);
			}

			gdi_surface::~gdi_surface()
			{
				::CGContextRelease(_context);
				delete []data;
			}

			CGContextRef gdi_surface::lock()
			{
				return _context;
			}

			void gdi_surface::unlock()
			{
				CGContextFlush(_context);
			}
		}
	}
}
