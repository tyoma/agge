#pragma once

#include <agge.text/font.h>

struct HFONT__;
typedef struct HFONT__ *HFONT;

namespace demo
{
	struct knuth_hash
	{
		size_t operator ()(int key) const throw() { return key * 2654435761; }
	};

	class font : public agge::font
	{
	public:
		static std::shared_ptr<font> create(int height, const wchar_t *typeface, bool bold, bool italic);

		HFONT native() const;

	private:
		typedef std::unordered_map<wchar_t, agge::uint16_t, knuth_hash> char2index;

	private:
		font(const metrics &m, std::shared_ptr<void> native);

		virtual agge::uint16_t get_glyph_index(wchar_t character) const;
		virtual const agge::glyph *load_glyph(agge::uint16_t index) const;
		virtual agge::pod_vector<kerning_pair> load_kerning() const;

	private:
		std::shared_ptr<void> _native;
		char2index _char2index;
	};		 
}
