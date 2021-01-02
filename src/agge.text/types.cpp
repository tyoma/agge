#include <agge.text/types.h>

#include <algorithm>

using namespace std;

namespace agge
{
	namespace
	{
		struct nc_compare
		{
			bool operator ()(char lhs, char rhs) const
			{	return toupper(lhs) < toupper(rhs);	}
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

	bool operator <(const font_descriptor &lhs, const font_descriptor &rhs)
	{
		return lhs.height < rhs.height ? true : rhs.height < lhs.height ? false :
			lhs.bold < rhs.bold ? true : rhs.bold < lhs.bold ? false :
			lhs.italic < rhs.italic ? true : rhs.italic < lhs.italic ? false :
			lhs.hinting < rhs.hinting ? true : rhs.hinting < lhs.hinting ? false :
			lexicographical_compare(lhs.family.begin(), lhs.family.end(), rhs.family.begin(), rhs.family.end(),
				nc_compare());
	}
}
