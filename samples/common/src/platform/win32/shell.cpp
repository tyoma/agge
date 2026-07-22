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

#include <samples/common/services.h>
#include <samples/common/shell.h>
#include <samples/common/smoothing.h>
#include <samples/common/timing.h>

#include "../../shell-inline.h"

#include <memory>
#include <stdexcept>
#include <stdio.h>
#include <tasker/ui_queue.h>
#include <tchar.h>
#include <windows.h>

using namespace std;

namespace
{
	const application::timings c_zero_timings = { };
	const int c_initial_width = 736;
	const int c_initial_height = 800;
	const auto c_prop_name = _T("window_proc");

	mt::milliseconds clock_ms()
	{
		static const double period = [] {
			LARGE_INTEGER f;
			::QueryPerformanceFrequency(&f);
			return 1000.0 / f.QuadPart;
		}();
		LARGE_INTEGER c;
		::QueryPerformanceCounter(&c);
		return mt::milliseconds(static_cast<long long>(period * c.QuadPart));
	}

	class desktop_services : public services
	{
		virtual stream *open_file(const char *path) override
		{
			class file_stream : public stream
			{
			public:
				file_stream(const char *path)
					: _stream(fopen(path, "rb"), &fclose)
				{	}

				virtual void read(void *buffer, size_t size)
				{	fread(buffer, 1, size, _stream.get());	}

			private:
				shared_ptr<FILE> _stream;
			};

			return new file_stream(path);
		}
	};

	class application_window
	{
	public:
		application_window(application &application_)
			: _window(::CreateWindow(_T("#32770"), NULL, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 0, 0, NULL, NULL, NULL,
				NULL), &::DestroyWindow),
			_bitmap(1, 1), _application(application_), _queue(&clock_ms)
		{
			if (!_window)
				throw std::runtime_error("Cannot create window!");
			::SetProp(hwnd(), c_prop_name, static_cast<HANDLE>(this));
			_original_wndproc = ::SetWindowLongPtr(hwnd(), GWLP_WNDPROC, reinterpret_cast<uintptr_t>(&window_proc_s));
			::SetWindowPos(hwnd(), HWND_TOP, 0, 0, c_initial_width, c_initial_height, SWP_NOMOVE | SWP_NOZORDER);
			schedule_invalidation();
		}

		void update_text()
		{
			TCHAR caption[1000] = { };
			RECT rc;
			const auto &t = _timings_smoothing.get();

			::GetClientRect(hwnd(), &rc);

			_stprintf_s(caption, _T("Total (%dx%d): %.2fms, clear: %.2fms, stroking: %.2fms, raster: %.2fms, render: %.2fms, blitting: %.2fms"), rc.right, rc.bottom,
				t.clearing + t.stroking + t.rasterization + t.rendition,
				t.clearing,
				t.stroking,
				t.rasterization,
				t.rendition,
				t.blitting);
			::SetWindowText(hwnd(), caption);
		}

	private:
		HWND hwnd() const
		{	return static_cast<HWND>(_window.get());	}

		static uintptr_t __stdcall window_proc_s(HWND hwnd, unsigned int message, uintptr_t wparam, uintptr_t lparam)
		{	return static_cast<application_window *>(::GetProp(hwnd, c_prop_name))->window_proc(message, wparam, lparam);	}

		uintptr_t window_proc(unsigned int message, uintptr_t wparam, uintptr_t lparam)
		{
			switch (message)
			{
			case WM_SIZE:
				if (LOWORD(lparam) && HIWORD(lparam))
				{
					_bitmap.resize(LOWORD(lparam), HIWORD(lparam));
					_application.resize(LOWORD(lparam), HIWORD(lparam));
					::InvalidateRect(hwnd(), NULL, FALSE);
				}
				return 0;

			case WM_NCDESTROY:
				::SetWindowLongPtr(hwnd(), GWLP_WNDPROC, _original_wndproc);
				::RemoveProp(hwnd(), _T("window_proc"));
				return 0;

			case WM_CLOSE:
				PostQuitMessage(0);
				return 0;

			case WM_ERASEBKGND:
				return TRUE;

			default:
				return ::CallWindowProc(reinterpret_cast<WNDPROC>(_original_wndproc), hwnd(), message, wparam, lparam);

			case WM_PAINT:
				PAINTSTRUCT ps;
				long long counter;
				application::timings t = {};

				_application.draw(_bitmap, t);

				stopwatch(counter);
				::BeginPaint(hwnd(), &ps);
				_bitmap.blit(ps.hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top);
				::EndPaint(hwnd(), &ps);
				t.blitting += stopwatch(counter);
				_timings_smoothing.add(t);
				return 0;
			}
		}

		void schedule_invalidation()
		{
			::InvalidateRect(hwnd(), NULL, TRUE);
			update_text();
			_queue.schedule([this] {
				schedule_invalidation();
			}, mt::milliseconds(20));
		}

	private:
		shared_ptr<void> _window;
		uintptr_t _original_wndproc;
		platform_bitmap _bitmap;
		application &_application;
		smoothing<application::timings> _timings_smoothing;
		tasker::ui_queue _queue;
	};
}

int main()
{
	::SetProcessDPIAware();

	desktop_services s;
	unique_ptr<application> app(agge_create_application(s));
	application_window dialog(*app);
	MSG msg;

	while (::GetMessage(&msg, NULL, 0, 0))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}
