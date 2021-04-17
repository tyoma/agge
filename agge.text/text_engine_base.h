#pragma once

#include "font.h"
#include "font_factory.h"

#include <agge.text/types.h>

namespace agge
{
	class text_engine_base : public font_factory, noncopyable
	{
	public:
		struct loader;
		class offset_conv;

	public:
		explicit text_engine_base(loader &loader_, unsigned collection_cycles = 5);
		virtual ~text_engine_base();

		void collect();
		virtual font::ptr create_font(const font_descriptor &descriptor);

	private:
		class cached_outline_accessor;

		struct font_key_hasher
		{
			size_t operator ()(const font_descriptor &key) const;
		};

		typedef hash_map<font_descriptor, std::pair<font::ptr, unsigned /*generation*/>, font_key_hasher> fonts_cache;
		typedef hash_map<font_descriptor, weak_ptr<font::accessor>, font_key_hasher> scalabale_fonts_cache;

	private:
		std::pair<font::accessor_ptr, real_t> create_font_accessor(font_descriptor fk);
		void on_released(const fonts_cache::value_type *entry, font *font_);
		void destroy(font *font_) throw();

		virtual void on_before_removed(font * /*font_*/) throw() {	}

	private:
		loader &_loader;
		const unsigned _collection_cycles;
		fonts_cache _fonts;
		scalabale_fonts_cache _scalable_fonts;
	};

	struct text_engine_base::loader
	{
		virtual font::accessor_ptr load(const font_descriptor &descriptor) = 0;
	};

	class text_engine_base::offset_conv
	{
	public:
		offset_conv(const glyph::path_iterator &source, real_t dx, real_t dy);

		void rewind(unsigned id);
		int vertex(real_t *x, real_t *y);

	private:
		glyph::path_iterator _source;
		real_t _dx, _dy;
	};
}
