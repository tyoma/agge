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

	public:
		virtual ~font();

		const glyph *get_glyph(wchar_t character) const;
		real_t get_kerning(wchar_t former, wchar_t latter) const;

	protected:
		struct kerning_pair;
		typedef std::unordered_map<uint16_t, const glyph *> glyphs_cache_t;

	private:
		virtual uint16_t get_glyph_index(wchar_t character) const = 0;
		virtual const glyph *load_glyph(uint16_t index) const = 0;
		virtual pod_vector<kerning_pair> load_kerning() const = 0;

	private:
		mutable glyphs_cache_t _glyphs;
	};
}
