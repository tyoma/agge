//	Copyright (c) 2011-2014 by Artem A. Gevorkyan (gevorkyan.org)
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.

#include "MainDialog.h"

#include <aggx/win32_bitmap.h>

#include <stdexcept>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>

using namespace std;

namespace
{
	const int c_initial_width = 736;
	const int c_initial_height = 800;
}

MainDialog::MainDialog(const render_method &render)
	: _window(::CreateWindow(_T("#32770"), NULL, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, c_initial_width, c_initial_height, NULL, NULL, NULL, NULL)),
		_render(render), _cycles(0), _total_clearing(0), _total_rasterization(0), _total_rendition(0)
{
	if (!_window)
		throw std::runtime_error("Cannot create window!");
	::SetProp(_window, _T("windowProc"), static_cast<HANDLE>(this));
	_previousWindowProc = ::SetWindowLongPtr(_window, GWLP_WNDPROC, reinterpret_cast<uintptr_t>(&windowProcProxy));

	::SetTimer(_window, 1, 10, NULL);

	RECT rc;

	::GetClientRect(_window, &rc);

	_bitmap.reset(new aggx::bitmap(rc.right, rc.bottom));

	Update();
}

MainDialog::~MainDialog()
{
	destroy();
}

void MainDialog::onDestroy()
{
	::SetWindowLongPtr(_window, GWLP_WNDPROC, _previousWindowProc);
	::RemoveProp(_window, _T("windowProc"));
	_window = NULL;
}

void MainDialog::destroy()
{
	if (_window)
	{
		::DestroyWindow(_window);
	}
}

uintptr_t __stdcall MainDialog::windowProcProxy(HWND hwnd, unsigned int message, uintptr_t wparam, uintptr_t lparam)
{
	return static_cast<MainDialog *>(::GetProp(hwnd, _T("windowProc")))->windowProc(message, wparam, lparam);
}

uintptr_t MainDialog::windowProc(unsigned int message, uintptr_t wparam, uintptr_t lparam)
{
	switch (message)
	{
	case WM_SIZE:
		{
			if (LOWORD(lparam) && HIWORD(lparam))
			{
				_bitmap.reset();
				_bitmap.reset(new aggx::bitmap(LOWORD(lparam), HIWORD(lparam)));
				Update();
				::InvalidateRect(_window, NULL, FALSE);
			}
		}
		return 0;

	case WM_CLOSE:
		{
			PostQuitMessage(0);
			destroy();
		}
		return 0;

	case WM_PAINT:
		{
			PAINTSTRUCT ps;

			::BeginPaint(_window, &ps);
			if (_bitmap.get())
				_bitmap->blit(ps.hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top);
			::EndPaint(_window, &ps);
		}
		return 0;

	case WM_ERASEBKGND:
		return TRUE;

	case WM_TIMER:
		Update();
		::InvalidateRect(_window, NULL, TRUE);
		return 0;

	default:
		return ::CallWindowProc(reinterpret_cast<WNDPROC>(_previousWindowProc), _window, message, wparam, lparam);
	}
}


void MainDialog::Update()
{
	double clearing = 0, rasterization = 0, rendition = 0;

	if (_bitmap.get())
	{
		_render(ref(*_bitmap), ref(clearing), ref(rasterization), ref(rendition));
	}

	_total_clearing += clearing, _total_rasterization += rasterization, _total_rendition += rendition;

	if (0 == (++_cycles & 0x3F))
	{
		TCHAR caption[1000] = { 0 };
		RECT rc;

		::GetClientRect(_window, &rc);

		_stprintf(caption, _T("Total (%dx%d): %gms, clear: %gms, raster: %gms, render : %gms"), rc.right, rc.bottom,
			(_total_clearing + _total_rasterization + _total_rendition) / _cycles, _total_clearing / _cycles, _total_rasterization / _cycles, _total_rendition / _cycles);
		_cycles = 0;
		_total_clearing = 0;
		_total_rasterization = 0;
		_total_rendition = 0;
		::SetWindowText(_window, caption);

		::InvalidateRect(_window, NULL, TRUE);
	}
}
