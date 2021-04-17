#include <agge.text/text_engine_base.h>

#include <agge.text/functional.h>

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

		virtual glyph_index_t get_glyph_index(codepoint_t character) const
		{	return _underlying->get_glyph_index(character);	}

		virtual glyph::outline_ptr load_glyph(glyph_index_t index, glyph::glyph_metrics &m) const
		{
			glyphs::iterator i = _glyphs.find(index);

			if (_glyphs.end() == i)
			{
				glyph::outline_ptr o = _underlying->load_glyph(index, m);
					
				i = _glyphs.insert(make_pair(index, make_pair(o, m))).first;
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
	{	}

	void text_engine_base::collect()
	{	}

	font::ptr text_engine_base::create_font(const font_descriptor &descriptor)
	{
		fonts_cache::iterator i = _fonts.find(descriptor);

		if (_fonts.end() != i)
			return i->second.first;

		pair<font::accessor_ptr, real_t> acc = create_font_accessor(descriptor);
		font_descriptor descriptor_normalized = acc.first->get_descriptor();
		if (hint_none == descriptor.hinting)
			descriptor_normalized.height = descriptor.height;
		pair<fonts_cache::iterator, bool> inserted = _fonts.insert(make_pair(descriptor_normalized,
			make_pair(font::ptr(), 0u)));

		if (inserted.second)
		{
			inserted.first->second.first = font::ptr(new font(acc.first, acc.second), bind(&text_engine_base::on_released, this,
				&*inserted.first, _1));
			return inserted.first->second.first;
		}
		return create_font(descriptor_normalized);
	}

	pair<font::accessor_ptr, real_t> text_engine_base::create_font_accessor(font_descriptor fd)
	{
		if (hint_none != fd.hinting)
			return make_pair(_loader.load(fd), 1.0f);

		const real_t factor = static_cast<real_t>(fd.height) / c_rescalable_height;
		const int height = fd.height;

		fd.height = c_rescalable_height;

		pair<scalabale_fonts_cache::iterator, bool> i = _scalable_fonts.insert(make_pair(fd, weak_ptr<font::accessor>()));

		if (i.second || i.first->second.expired())
		{
			font::accessor_ptr a_ = _loader.load(fd);
			fd.height = height;
			font::accessor_ptr a(new cached_outline_accessor(fd, a_));

			i.first->second = a;
			return make_pair(a, factor);
		}
		return make_pair(i.first->second.lock(), factor);
	}

	void text_engine_base::on_released(const fonts_cache::value_type * /*entry*/, font *font_)
	{
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
