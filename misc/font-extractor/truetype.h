#pragma once

#include <agge/types.h>
#include <unordered_map>

namespace std { namespace tr1 { } using namespace tr1; }

namespace agge
{
	namespace truetype
	{
		struct segment
		{
			enum segment_type { line = 1, qspline = 2, cspline = 3 } type;
			std::vector< point<int> > points;
		};

		struct poly
		{
			point<int> start;
			std::vector<segment> segments;
		};

		struct glyph_metrics
		{
			int advance_x, advance_y;
		};

		struct glyph
		{
			glyph_metrics metrics;
			std::vector<poly> polygons;
		};

		struct font
		{
			std::unordered_map<wchar_t, uint16_t> char_to_glyph;
			std::vector<glyph> glyphs;
		};



		template <typename ArchiveT>
		inline void serialize(ArchiveT &archive, segment &data)
		{	archive(data.type), archive(data.points);	}

		template <typename ArchiveT>
		inline void serialize(ArchiveT &archive, poly &data)
		{	archive(data.start), archive(data.segments);	}

		template <typename ArchiveT>
		inline void serialize(ArchiveT &archive, glyph_metrics &data)
		{	archive(data.advance_x), archive(data.advance_y);	}

		template <typename ArchiveT>
		inline void serialize(ArchiveT &archive, glyph &data)
		{	archive(data.metrics), archive(data.polygons);	}

		template <typename ArchiveT>
		inline void serialize(ArchiveT &archive, font &data)
		{	archive(data.char_to_glyph), archive(data.glyphs);	}

		template <typename ArchiveT>
		void serialize(ArchiveT &archive, segment::segment_type &data)
		{	archive(reinterpret_cast<int &>(data));	}
	}

	template <typename ArchiveT, typename CoordT>
	inline void serialize(ArchiveT &archive, point<CoordT> &data)
	{	archive(data.x), archive(data.y);	}
}
