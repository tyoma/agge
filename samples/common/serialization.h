#pragma once

#include "truetype.h"

namespace truetype
{
	template <typename ArchiveT>
	inline void serialize(ArchiveT &archive, segment &data)
	{	archive(data.type), archive(data.points);	}

	template <typename ArchiveT>
	inline void serialize(ArchiveT &archive, poly &data)
	{	archive(data.start), archive(data.segments);	}

	template <typename ArchiveT>
	inline void serialize(ArchiveT &archive, glyph::glyph_metrics &data)
	{	archive(data.advance_x), archive(data.advance_y);	}

	template <typename ArchiveT>
	inline void serialize(ArchiveT &archive, glyph &data)
	{	archive(data.metrics), archive(data.polygons);	}

	template <typename ArchiveT>
	inline void serialize(ArchiveT &archive, font::font_metrics &data)
	{	archive(data.ascent), archive(data.descent), archive(data.leading);	}

	template <typename ArchiveT>
	inline void serialize(ArchiveT &archive, font &data)
	{	archive(data.metrics), archive(data.char_to_glyph), archive(data.glyphs);	}

	template <typename ArchiveT>
	void serialize(ArchiveT &archive, segment::segment_type &data)
	{	archive(reinterpret_cast<int &>(data));	}
}

namespace agge
{
	template <typename ArchiveT, typename CoordT>
	inline void serialize(ArchiveT &archive, point<CoordT> &data)
	{	archive(data.x), archive(data.y);	}
}
