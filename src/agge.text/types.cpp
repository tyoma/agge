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

		struct nc_equal
		{
			bool operator ()(char lhs, char rhs) const
			{	return toupper(lhs) == toupper(rhs);	}
		};
	}

	full_alignment full_alignment::create(text_alignment halign_, text_alignment valign_)
	{
		full_alignment a;

		return a.halign = halign_, a.valign = valign_, a;
	}

	font_descriptor font_descriptor::create(const string &family, int height, font_weight weight, bool italic,
		font_hinting hinting)
	{
		font_descriptor d = {};

		d.family = family;
		d.height = height;
		d.weight = weight;
		d.italic = italic;
		d.hinting = hinting;
		return d;
	}

	bool operator <(const font_descriptor &lhs, const font_descriptor &rhs)
	{
		return lhs.height < rhs.height ? true : rhs.height < lhs.height ? false :
			lhs.weight < rhs.weight ? true : rhs.weight < lhs.weight ? false :
			lhs.italic < rhs.italic ? true : rhs.italic < lhs.italic ? false :
			lhs.hinting < rhs.hinting ? true : rhs.hinting < lhs.hinting ? false :
			lexicographical_compare(lhs.family.begin(), lhs.family.end(), rhs.family.begin(), rhs.family.end(),
				nc_compare());
	}

	bool operator ==(const font_descriptor& lhs, const font_descriptor& rhs)
	{
		return (lhs.height == rhs.height)
			& (lhs.weight == rhs.weight)
			& (lhs.italic == rhs.italic)
			& (lhs.hinting == rhs.hinting)
			& (lhs.family.size() == rhs.family.size() && equal(lhs.family.begin(), lhs.family.end(), rhs.family.begin(), nc_equal()));
	}
}
