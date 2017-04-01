#pragma once

#include <agge/bitmap.h>
#include <agge/platform/win32/bitmap.h>

#include <memory>

struct HWND__;
typedef HWND__ *HWND;

typedef agge::bitmap<agge::pixel32, agge::platform::raw_bitmap> bitmap;

struct Timings
{
	double clearing;
	double stroking;
	double rasterization;
	double rendition;
	double blitting;
};

struct Drawer
{
	virtual void draw(bitmap &surface, Timings &timings) = 0;
	virtual void resize(int width, int height);
};

class MainDialog
{
public:
	MainDialog(Drawer &drawer);
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
	bitmap _bitmap;
	Drawer &_drawer;

	int _cycles;
	Timings _timings;
};



inline void Drawer::resize(int /*width*/, int /*height*/)
{	}
