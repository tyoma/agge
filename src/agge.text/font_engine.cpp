#include <agge.text/font_engine.h>

#include <cctype>
#include <string>

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

	struct font_engine::font_key
	{
		wstring typeface;
		unsigned height : 20;
		unsigned bold : 1;
		unsigned italic : 1;

		bool operator ==(const font_key &rhs) const
		{
			return typeface.size() == rhs.typeface.size()
				&& height == rhs.height
				&& bold == rhs.bold
				&& italic == rhs.italic
				&& equal(typeface.begin(), typeface.end(), rhs.typeface.begin(), nc_compare());
		}
	};

	struct font_engine::font_key_hasher
	{
		size_t operator ()(const font_engine::font_key &/*key*/) const
		{
			return 1;
		}
	};

	font_engine::font_engine(loader &/*loader_*/)
		: _fonts(new fonts_cache)
	{
	}

	font::ptr font_engine::create_font(const wchar_t *typeface, int height, bool bold, bool italic)
	{
		font_key key = { typeface };
		
		key.height = height;
		key.bold = !!bold;
		key.italic = !!italic;

		fonts_cache::iterator i = _fonts->find(key);

		if (i == _fonts->end())
		{
			struct accessor : font::accessor
			{
				virtual font::metrics get_metrics() const
				{
					return font::metrics();
				}

				virtual uint16_t get_glyph_index(wchar_t /*character*/) const
				{
					return 0;
				}

				virtual bool load_glyph(uint16_t /*index*/, glyph::glyph_metrics &/*m*/, glyph::outline_storage &/*o*/) const
				{
					return false;
				}
			};

			font::accessor_ptr fa((font::accessor *)new accessor);
			font::ptr f(new font(fa));
			
			_fonts->insert(key, f, i);
			i->second.reset(new font(fa));
		}
		return i->second;
	}
}
