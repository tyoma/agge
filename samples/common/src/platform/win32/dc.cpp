#include <samples/common/platform/win32/dc.h>

#include <functional>
#include <windows.h>

using namespace std;
using namespace std::placeholders;

dc::dc(platform_bitmap *surface)
	: _dc(::CreateCompatibleDC(NULL)), _bitmap_selector(surface ? select(surface->native()) : 0)
{	}

dc::~dc()
{
	_bitmap_selector.reset();
	::DeleteDC(_dc);
}

dc::handle dc::select(HGDIOBJ obj)
{	return handle(::SelectObject(_dc, obj), bind(&::SelectObject, _dc, _1));	}
