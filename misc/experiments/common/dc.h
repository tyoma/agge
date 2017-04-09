#pragma once

#include <agge/bitmap.h>
#include <agge/platform/win32/bitmap.h>
#include <agge/types.h>
#include <agge.text/shared_ptr.h>

typedef agge::bitmap<agge::pixel32, agge::platform::raw_bitmap> bitmap;
struct HDC__;
typedef struct HDC__ *HDC;
typedef void *HGDIOBJ;

namespace common
{
	class dc : agge::noncopyable
	{
	public:
		typedef agge::shared_ptr<void> handle;

	public:
		explicit dc(bitmap *surface = 0);
		~dc();

		operator HDC() const;

		handle select(HGDIOBJ obj);

	private:
		HDC _dc;
		handle _bitmap_selector;
	};



	inline dc::operator HDC() const
	{	return _dc;	}
}
