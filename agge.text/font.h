#pragma once

#include <agge/pod_vector.h>
#include <memory>
#include <unordered_map>

namespace std { namespace tr1 {} using namespace tr1; }

namespace agge
{
	class glyph;

	class font : noncopyable
	{
	public:
		typedef std::shared_ptr<font> ptr;

		struct metrics
		{
			real_t ascent;
			real_t descent;
			real_t leading;
		};

	public:
		virtual ~font();

		metrics get_metrics() const;

		const glyph *get_glyph(wchar_t character) const;
		real_t get_kerning(wchar_t former, wchar_t latter) const;

	protected:
		struct kerning_pair;
		typedef std::unordered_map<uint16_t, const glyph *> glyphs_cache_t;

	protected:
		font(const metrics &metrics_);

	private:
		virtual uint16_t get_glyph_index(wchar_t character) const = 0;
		virtual const glyph *load_glyph(uint16_t index) const = 0;
		virtual pod_vector<kerning_pair> load_kerning() const = 0;

	private:
		const metrics _metrics;
		mutable glyphs_cache_t _glyphs;
	};
}
