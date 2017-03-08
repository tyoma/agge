#include <agge.text/font_engine.h>

#include <cctype>
#include <string>

using namespace std;

namespace agge
{
	namespace
	{
		enum {
			c_rescalable_height = 1000,
		};

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
		unsigned grid_fit : 3;

		bool operator ==(const font_key &rhs) const
		{
			return typeface.size() == rhs.typeface.size()
				&& height == rhs.height
				&& bold == rhs.bold
				&& italic == rhs.italic
				&& grid_fit == rhs.grid_fit
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

	font_engine::font_engine(loader &loader_)
		: _loader(loader_), _fonts(new fonts_cache), _scalable_fonts(new scalabale_fonts_cache)
	{
	}

	font::ptr font_engine::create_font(const wchar_t *typeface, int height, bool bold, bool italic, grid_fit gf)
	{
		font_key key = { typeface };
		
		key.height = height;
		key.bold = !!bold;
		key.italic = !!italic;
		key.grid_fit = gf;

		fonts_cache::iterator i = _fonts->find(key);

		if (i == _fonts->end())
		{
			real_t factor = 1.0f;
			font::accessor_ptr a;

			if (gf_none == gf)
			{
				scalabale_fonts_cache::iterator i;
				font_key skey = key;
				
				factor = static_cast<real_t>(height) / c_rescalable_height;
				skey.height = c_rescalable_height;
				if (_scalable_fonts->insert(skey, font::accessor_ptr(), i))
					a = i->second = _loader.load(typeface, skey.height, bold, italic, gf);
				else
					a = i->second;
			}
			else
			{
				a = _loader.load(typeface, height, bold, italic, gf);
			}

			font::ptr f(new font(a, factor));
			
			_fonts->insert(key, f, i);
		}
		return i->second;
	}
}
