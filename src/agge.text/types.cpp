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

	font_descriptor font_descriptor::create(const string &family, int height, bool bold, bool italic,
		font_hinting hinting_)
	{
		font_descriptor d = {};

		d.family = family;
		d.height = height;
		d.bold = bold;
		d.italic = italic;
		d.hinting = hinting_;
		return d;
	}

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
