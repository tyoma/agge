#pragma once

#include "font.h"
#include "layout_builder.h"
#include "richtext.h"
#include "tools.h"
#include "utf8.h"

#include <agge/math.h>
#include <agge/tools.h>

namespace agge
{
	class layout : noncopyable
	{
	public:
		typedef text_lines_container_t::const_iterator const_iterator;

	public:
		template <typename LimitProcessorT, typename FontFactoryT>
		void process(const richtext_t &text, LimitProcessorT limit_processor, FontFactoryT &font_factory_);

		box_r get_box() const;

		const_iterator begin() const;
		const_iterator end() const;

	private:
		positioned_glyphs_container_t _glyphs;
		glyph_runs_container_t _glyph_runs;
		text_lines_container_t _text_lines;
		box_r _box;
	};



	template <typename LimiterT, typename FontFactoryT>
	inline void layout::process(const richtext_t &text, LimiterT limiter, FontFactoryT &font_factory_)
	{
		_glyphs.resize(static_cast<count_t>(text.size()));
		_box = zero();

		layout_builder builder(_glyphs, _glyph_runs, _text_lines);

		for (richtext_t::const_iterator range = text.ranges_begin(), ranges_end = text.ranges_end(); range != ranges_end;
			++range)
		{
			const font::ptr font_ = font_factory_.create_font(range->get_annotation().basic);

			builder.begin_style(font_);
			limiter.begin_style(static_cast<const layout_builder &>(builder));
			for (std::string::const_iterator i = range->begin(), end = range->end(); i != end; )
			{
				if (eat_lf(i))
				{
					builder.break_current_line();
					limiter.new_line();
					continue;
				}

				std::string::const_iterator next = i;
				const glyph &g = *font_->get_glyph_for_codepoint(utf8::next(next, end));

				if (!limiter.add_glyph(builder, g.index, g.metrics.advance_x, i, next, end))
				{
					_text_lines.clear();
					return;
				}
			}
		}
		builder.break_current_line();
		_text_lines.pop_back();
		for (auto i = _text_lines.begin(); i != _text_lines.end(); ++i)
		{
			if (i->extent > _box.w)
				_box.w = i->extent;
			_box.h = i->offset.dy + i->descent;
		}
	}

	inline box_r layout::get_box() const
	{	return _box;	}

	inline layout::const_iterator layout::begin() const
	{	return _text_lines.begin();	}

	inline layout::const_iterator layout::end() const
	{	return _text_lines.end();	}
}
