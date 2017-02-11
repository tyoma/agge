#pragma once

#include <agge/pod_vector.h>

namespace agge
{
	class glyph;

	class font
	{
	public:
		virtual ~font() { }

		const glyph *get_glyph(wchar_t character) const;
		real_t get_kerning(wchar_t former, wchar_t latter) const;

	private:
		struct kerning_pair;

	private:
		virtual uint16_t get_glyph_index(wchar_t character) const = 0;
		virtual const glyph *load_glyph(uint16_t index) const = 0;
		virtual pod_vector<kerning_pair> load_kerning() const = 0;
	};
}
