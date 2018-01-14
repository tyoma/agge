#include "surface.h"

#include <windows.h>
#include <stdexcept>

using namespace std;

namespace agge
{
	namespace platform
	{
		namespace tests
		{
			gdi_surface::gdi_surface(count_t width_, count_t height_)
				: width(width_), height(height_), _context(::CreateCompatibleDC(0)), _previous_bitmap(0)
			{
				BITMAPINFO bi = { };

				bi.bmiHeader.biSize           = sizeof(BITMAPINFOHEADER);
				bi.bmiHeader.biWidth          = width;
				bi.bmiHeader.biHeight         = -(int)height;
				bi.bmiHeader.biPlanes         = 1;
				bi.bmiHeader.biBitCount       = 32;
				bi.bmiHeader.biCompression    = BI_RGB;
				bi.bmiHeader.biSizeImage      = 0;

				_bitmap = ::CreateDIBSection(_context, &bi, DIB_RGB_COLORS, &data, NULL, 0);
			}

			gdi_surface::~gdi_surface()
			{
				unlock();
				::DeleteObject(_bitmap);
				::DeleteDC(_context);
			}

			HDC gdi_surface::lock()
			{
				if (_previous_bitmap)
					throw logic_error("Unexpected test surface locking!");
				_previous_bitmap = (HBITMAP)::SelectObject(_context, _bitmap);
				return _context;
			}

			void gdi_surface::unlock()
			{
				if (_previous_bitmap)
				{
					::SelectObject(_context, _previous_bitmap);
					_previous_bitmap = 0;
				}
			}

			bool gdi_surface::is_valid_handle(HBITMAP hbitmap) const
			{
				HGDIOBJ previous = ::SelectObject(_context, hbitmap);
				bool valid = !!previous;

				if (previous)
					::SelectObject(_context, previous);
				return valid;
			}
		}
	}
}
