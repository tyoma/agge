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
#include <samples/common/timing.h>

#include "../../shell-inline.h"

#include <memory>
#include <stdexcept>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>

using namespace std;

namespace
{
	const application::timings c_zero_timings = { };
	const int c_initial_width = 736;
	const int c_initial_height = 800;

	class desktop_services : public services
	{
		virtual stream *open_file(const char *path);
	};

	class MainDialog
	{
	public:
		MainDialog(application &application_);
		~MainDialog();

		void UpdateText();

		static void PumpMessages();

	private:
		static uintptr_t __stdcall windowProcProxy(HWND hwnd, unsigned int message, uintptr_t wparam, uintptr_t lparam);
		uintptr_t windowProc(unsigned int message, uintptr_t wparam, uintptr_t lparam);

		void onDestroy();

		void destroy();

	private:
		HWND _window;
		uintptr_t _previousWindowProc;
		platform_bitmap _bitmap;
		application &_application;

		int _cycles;
		application::timings _timings;
	};



	stream *desktop_services::open_file(const char *path)
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


	MainDialog::MainDialog(application &application_)
		: _window(::CreateWindow(_T("#32770"), NULL, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, c_initial_width, c_initial_height, NULL, NULL, NULL, NULL)),
			_bitmap(1, 1), _application(application_), _cycles(0), _timings(c_zero_timings)
	{
		if (!_window)
			throw std::runtime_error("Cannot create window!");
		::SetProp(_window, _T("windowProc"), static_cast<HANDLE>(this));
		_previousWindowProc = ::SetWindowLongPtr(_window, GWLP_WNDPROC, reinterpret_cast<uintptr_t>(&windowProcProxy));

		RECT rc;

		::GetClientRect(_window, &rc);

		_bitmap.resize(rc.right, rc.bottom);
		_application.resize(rc.right, rc.bottom);

		UpdateText();

		::SetTimer(_window, 1, 1, 0);
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
					_bitmap.resize(LOWORD(lparam), HIWORD(lparam));
					_application.resize(LOWORD(lparam), HIWORD(lparam));
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
				long long counter;

				_application.draw(_bitmap, _timings);

				stopwatch(counter);
				::BeginPaint(_window, &ps);
				_bitmap.blit(ps.hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top);
				::EndPaint(_window, &ps);
				_timings.blitting += stopwatch(counter);
				++_cycles;

				UpdateText();
			}
			return 0;

		case WM_ERASEBKGND:
			return TRUE;

		case WM_TIMER:
			UpdateText();
			::InvalidateRect(_window, NULL, TRUE);
			return TRUE;

		default:
			return ::CallWindowProc(reinterpret_cast<WNDPROC>(_previousWindowProc), _window, message, wparam, lparam);
		}
	}


	void MainDialog::UpdateText()
	{
		if (_cycles > 100)
		{
			TCHAR caption[1000] = { };
			RECT rc;

			::GetClientRect(_window, &rc);

			_stprintf_s(caption, _T("Total (%dx%d): %gms, clear: %gms, stroking: %gms, raster: %gms, render: %gms, blitting: %gms"), rc.right, rc.bottom,
				(_timings.clearing + _timings.rasterization + _timings.rendition) / _cycles,
				_timings.clearing / _cycles,
				_timings.stroking / _cycles,
				_timings.rasterization / _cycles,
				_timings.rendition / _cycles,
				_timings.blitting / _cycles);
			_cycles = 0;
			_timings = c_zero_timings;
			::SetWindowText(_window, caption);

			::InvalidateRect(_window, NULL, TRUE);
		}
	}

	void MainDialog::PumpMessages()
	{
		MSG msg;

		while (::GetMessage(&msg, NULL, 0, 0))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
}

int main()
{
	::SetProcessDPIAware();

	desktop_services s;
	auto_ptr<application> app(agge_create_application(s));
	MainDialog dialog(*app);

	MainDialog::PumpMessages();
}
