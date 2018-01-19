#include <agge/platform/bitmap.h>

#include <agge/tools.h>

#include <memory>
#include <windows.h>

using namespace std;

namespace agge
{
	namespace platform
	{
		namespace
		{
			class dc : noncopyable
			{
			public:
				dc()
					: _dc(::CreateCompatibleDC(0))
				{	}

				~dc()
				{	::DeleteDC(_dc);	}

				operator HDC() const
				{	return _dc;	}

			private:
				HDC _dc;
			};
		}

		raw_bitmap::~raw_bitmap()
		{	::DeleteObject(_native);	}

		void raw_bitmap::resize(count_t width, count_t height)
		{
			if (width > _allocated_width || height > _allocated_height)
			{
				dc memdc;

				const count_t max_width = agge_max(width + _extra_pixels, _allocated_width);
				const count_t max_height = agge_max(height, _allocated_height);
				const count_t stride = calculate_stride(max_width);
				void *memory = 0;
				BITMAPINFO bi = { };

				bi.bmiHeader.biSize           = sizeof(BITMAPINFOHEADER);
				bi.bmiHeader.biWidth          = max_width;
				bi.bmiHeader.biHeight         = -(int)max_height;
				bi.bmiHeader.biPlanes         = 1;
				bi.bmiHeader.biBitCount       = (short)_bpp;
				bi.bmiHeader.biCompression    = BI_RGB;
				bi.bmiHeader.biSizeImage      = 0;

				HBITMAP native = ::CreateDIBSection(memdc, &bi, DIB_RGB_COLORS, &memory, 0, 0);

				if (!native)
					throw bad_alloc();

				::DeleteObject(_native);
				_native = native;
				_memory = static_cast<uint8_t *>(memory);
				_allocated_width = max_width;
				_allocated_height = max_height;
				_stride = stride;
			}
			_width = width, _height = height;
		}

		void raw_bitmap::blit(HDC hdc, int x, int y, count_t w, count_t h) const
		{
			dc memdc;

			HGDIOBJ prev = ::SelectObject(memdc, _native);
			::BitBlt(hdc, x, y, w, h, memdc, 0, 0, SRCCOPY);
			::SelectObject(memdc, prev);
		}
	}
}
