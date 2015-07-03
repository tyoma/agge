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

#pragma once

#include <functional>
#include <memory>

struct HWND__;
typedef HWND__ *HWND;

namespace aggx
{
	class bitmap;
}

class MainDialog
{
public:
	typedef std::function<void(aggx::bitmap &target, double &clearing, double &rasterization, double &rendition)> render_method;

public:
	MainDialog(const render_method &render);
	~MainDialog();

	void Update();

private:
	static uintptr_t __stdcall windowProcProxy(HWND hwnd, unsigned int message, uintptr_t wparam, uintptr_t lparam);
	uintptr_t windowProc(unsigned int message, uintptr_t wparam, uintptr_t lparam);

	void onDestroy();

	void destroy();

private:
	HWND _window;
	uintptr_t _previousWindowProc;
	std::auto_ptr<aggx::bitmap> _bitmap;
	const render_method _render;

	int _cycles;
	double _total_clearing, _total_rasterization, _total_rendition;
};
