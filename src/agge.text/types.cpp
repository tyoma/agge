#include <agge.text/types.h>

#include <algorithm>

using namespace std;

namespace agge
{
	namespace
	{
		struct nc_compare
		{
			bool operator ()(wchar_t lhs, wchar_t rhs) const
			{	return toupper(lhs) == toupper(rhs);	}
		};
	}

	font_descriptor::font_descriptor(const string &family_, int height_, bool bold_, bool italic_,
			font_hinting hinting_)
		: family(family_), height(height_), bold(bold_), italic(italic_), hinting(hinting_)
	{	}

	bool operator ==(const font_descriptor &lhs, const font_descriptor &rhs)
	{
		return lhs.family.size() == rhs.family.size()
			&& lhs.height == rhs.height
			&& lhs.bold == rhs.bold
			&& lhs.italic == rhs.italic
			&& lhs.hinting == rhs.hinting
			&& equal(lhs.family.begin(), lhs.family.end(), rhs.family.begin(), nc_compare());
	}
}
