#pragma once

#include <samples/common/shell.h>

#include <agge.text/shared_ptr.h>

struct HDC__;
typedef struct HDC__ *HDC;
typedef void *HGDIOBJ;

class dc : agge::noncopyable
{
public:
	typedef agge::shared_ptr<void> handle;

public:
	explicit dc(platform_bitmap *surface = 0);
	~dc();

	operator HDC() const;

	handle select(HGDIOBJ obj);

private:
	HDC _dc;
	handle _bitmap_selector;
};



inline dc::operator HDC() const
{	return _dc;	}
