#include <agge.text/text_engine.h>

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

		class cached_outline_accessor : public font::accessor, noncopyable
		{
		public:
			cached_outline_accessor(const font::accessor_ptr &underlying)
				: _underlying(underlying)
			{	}

		private:
			typedef hash_map< uint16_t, pair<glyph::outline_ptr, glyph::glyph_metrics> > glyphs;

		private:
			virtual font::metrics get_metrics() const
			{	return _underlying->get_metrics();	}

			virtual uint16_t get_glyph_index(wchar_t character) const
			{	return _underlying->get_glyph_index(character);	}

			virtual glyph::outline_ptr load_glyph(uint16_t index, glyph::glyph_metrics &m) const
			{
				glyphs::iterator i = _glyphs.find(index);

				if (_glyphs.end() == i)
				{
					glyph::outline_ptr o = _underlying->load_glyph(index, m);
					
					_glyphs.insert(index, make_pair(o, m), i);
				}
				m = i->second.second;
				return i->second.first;
			}

		private:
			const font::accessor_ptr _underlying;
			mutable glyphs _glyphs;
		};
	}

	struct text_engine_base::font_key
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

	struct text_engine_base::font_key_hasher
	{
		size_t operator ()(const text_engine_base::font_key &/*key*/) const
		{
			return 1;
		}
	};

	text_engine_base::text_engine_base(loader &loader_)
		: _loader(loader_), _fonts(new fonts_cache), _scalable_fonts(new scalabale_fonts_cache)
	{
	}

	font::ptr text_engine_base::create_font(const wchar_t *typeface, int height, bool bold, bool italic, grid_fit gf)
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
				scalabale_fonts_cache::iterator j;
				font_key skey = key;
				
				factor = static_cast<real_t>(height) / c_rescalable_height;
				skey.height = c_rescalable_height;
				if (_scalable_fonts->insert(skey, font::accessor_ptr(), j))
					j->second.reset(new cached_outline_accessor(_loader.load(typeface, skey.height, bold, italic, gf)));
				a = j->second;
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


	text_engine_base::offset_conv::offset_conv(const glyph::path_iterator &source, real_t dx, real_t dy)
		: _source(source), _dx(dx), _dy(dy)
	{	}

	void text_engine_base::offset_conv::rewind(unsigned /*id*/)
	{	}

	int text_engine_base::offset_conv::vertex(real_t *x, real_t *y)
	{
		int command = _source.vertex(x, y);

		*x += _dx, *y += _dy;
		return command;
	}
}
