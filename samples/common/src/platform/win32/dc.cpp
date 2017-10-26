#include <samples/common/platform/win32/dc.h>

#include <functional>
#include <windows.h>

using namespace std;
using namespace std::placeholders;

namespace
{
	HGDIOBJ select_object(HDC hdc, HGDIOBJ hobject)
	{	return ::SelectObject(hdc, hobject);	}
}

dc::dc(platform_bitmap *surface)
	: _dc(::CreateCompatibleDC(NULL)), _bitmap_selector(surface ? select(surface->native()) : agge::shared_ptr<void>())
{	}

dc::~dc()
{
	_bitmap_selector.reset();
	::DeleteDC(_dc);
}

dc::handle dc::select(HGDIOBJ obj)
{	return handle(select_object(_dc, obj), bind(&select_object, _dc, _1));	}
