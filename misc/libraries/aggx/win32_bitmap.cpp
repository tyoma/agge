#include "win32_bitmap.h"

#include <windows.h>
#include <stdexcept>

using namespace std;

namespace aggx
{
	bitmap::bitmap(unsigned width, unsigned height)
		: _width(width), _height(height), _stride((width | 0x03) + 1)
	{
		BITMAPINFO bi = { 0 };

		bi.bmiHeader.biSize           = sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth          = (width | 0x03) + 1;
		bi.bmiHeader.biHeight         = -(int)height;
		bi.bmiHeader.biPlanes         = 1;
		bi.bmiHeader.biBitCount       = 32;
		bi.bmiHeader.biCompression    = BI_RGB;
		bi.bmiHeader.biSizeImage      = 0;

		HDC hdc = ::CreateCompatibleDC(NULL);
		_handle = ::CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &_memory, NULL, 0);
		::DeleteDC(hdc);
		if (!_handle)
			throw bad_alloc();
	}

	bitmap::~bitmap()
	{
		::DeleteObject(_handle);
	}

	void bitmap::blit(HDC hdc, int x, int y, int w, int h)
	{
		HDC memdc = ::CreateCompatibleDC(hdc);

		HGDIOBJ prev = ::SelectObject(memdc, _handle);
		::BitBlt(hdc, x, y, w, h, memdc, x, y, SRCCOPY);
		::SelectObject(memdc, prev);
		::DeleteDC(memdc);
	}
}
