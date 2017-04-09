#include <samples/common/font_loader.h>

#if defined(_WIN32)
	#include <samples/common/platform/win32/font_accessor.h>
#else
#endif

using namespace agge;

font::accessor_ptr font_loader::load(const wchar_t *typeface, int height, bool bold, bool italic,
	font_engine_base::grid_fit grid_fit)
{	return font::accessor_ptr(new font_accessor(height, typeface, bold, italic, grid_fit));	}
