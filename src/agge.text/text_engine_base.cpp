#include <agge.text/text_engine_base.h>

#include <agge.text/functional.h>

using namespace std;

namespace agge
{
	namespace
	{
		enum {
			c_rescalable_height = 1000,
		};
	}

	class text_engine_base::cached_outline_accessor : public font::accessor, noncopyable
	{
	public:
		cached_outline_accessor(const font_descriptor &descriptor, const font::accessor_ptr &underlying)
			: _descriptor(descriptor), _underlying(underlying)
		{	}

	private:
		typedef hash_map< glyph_index_t, pair<glyph::outline_ptr, glyph::glyph_metrics> > glyphs;

	private:
		virtual font_descriptor get_descriptor() const
		{	return _descriptor;	}

		virtual font_metrics get_metrics() const
		{	return _underlying->get_metrics();	}

		virtual glyph_index_t get_glyph_index(wchar_t character) const
		{	return _underlying->get_glyph_index(character);	}

		virtual glyph::outline_ptr load_glyph(glyph_index_t index, glyph::glyph_metrics &m) const
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
		const font_descriptor _descriptor;
		const font::accessor_ptr _underlying;
		mutable glyphs _glyphs;
	};


	size_t text_engine_base::font_key_hasher::operator ()(const font_descriptor &/*key*/) const
	{	return 1;	}


	text_engine_base::text_engine_base(loader &loader_, unsigned collection_cycles)
		: _loader(loader_), _collection_cycles(collection_cycles)			
	{	}

	text_engine_base::~text_engine_base()
	{
		for (garbage_container::iterator i = _garbage.begin(); i != _garbage.end(); ++i)
			destroy(i->second.first);
	}

	void text_engine_base::collect()
	{
		for (garbage_container::iterator i = _garbage.begin(); i != _garbage.end(); )
			i = !--i->second.second ? destroy(i->second.first), _garbage.erase(i) : ++i;
	}

	font::ptr text_engine_base::create_font(const font_descriptor &descriptor)
	{
		fonts_cache::iterator i = _fonts.find(descriptor);

		if (_fonts.end() != i)
		{
			if (shared_ptr<font> f = i->second.lock())
				return f;

			garbage_container::const_iterator gi = _garbage.find(descriptor);

			if (_garbage.end() != gi)
			{
				font *f = gi->second.first;

				_garbage.erase(gi);

				font::ptr fptr(f, bind(&text_engine_base::on_released, this, &*i, _1));

				return i->second = fptr, fptr;
			}
		}

		pair<font::accessor_ptr, real_t> acc = create_font_accessor(descriptor);
		font_descriptor descriptor_normalized = acc.first->get_descriptor();
		if (hint_none == descriptor.hinting)
			descriptor_normalized.height = descriptor.height;
		pair<fonts_cache::iterator, bool> inserted = _fonts.insert(make_pair(descriptor_normalized, shared_ptr<font>()));

		if (inserted.second || inserted.first->second.expired())
		{
			font::ptr fptr(new font(acc.first, acc.second), bind(&text_engine_base::on_released, this, &*inserted.first, _1));

			return inserted.first->second = fptr, fptr;
		}
		return create_font(descriptor_normalized);
	}

	pair<font::accessor_ptr, real_t> text_engine_base::create_font_accessor(font_descriptor fd)
	{
		if (hint_none != fd.hinting)
			return make_pair(_loader.load(fd), 1.0f);

		scalabale_fonts_cache::iterator i;
		const real_t factor = static_cast<real_t>(fd.height) / c_rescalable_height;
		const int height = fd.height;

		fd.height = c_rescalable_height;
		if (_scalable_fonts.insert(fd, weak_ptr<font::accessor>(), i) || i->second.expired())
		{
			font::accessor_ptr a_ = _loader.load(fd);
			fd.height = height;
			font::accessor_ptr a(new cached_outline_accessor(fd, a_));

			i->second = a;
			return make_pair(a, factor);
		}
		return make_pair(i->second.lock(), factor);
	}

	void text_engine_base::on_released(const fonts_cache::value_type *entry, font *font_)
	{
		garbage_container::iterator i ;

		if (_collection_cycles)
			_garbage.insert(entry->first, make_pair(font_, _collection_cycles), i);
		else
			destroy(font_);
	}

	void text_engine_base::destroy(font *font_) throw()
	{
		on_before_removed(font_);
		delete font_;
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
