#pragma once

#include "font.h"
#include "layout_primitives.h"
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
		class manipulator;
		struct state;
		typedef std::vector<text_line> text_lines_container_t;
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

	struct layout::state
	{
		size_t next, runs_size;
		real_t extent;

		bool operator !() const;
		bool operator <(const state &rhs) const;
	};

	class layout::manipulator
	{
	public:
		manipulator(positioned_glyphs_container_t &glyphs, glyph_runs_container_t &glyph_runs,
			layout::text_lines_container_t &text_lines);

		void begin_style(const font::ptr &font_);

		void append_glyph(glyph_index_t index, real_t advance);
		void trim_current_line(const layout::state &at);
		bool break_current_line();
		void break_current_line(const layout::state &at, const layout::state &resume_at);
		const layout::state &get_state() const;

	private:
		void set_current(const font::ptr &font_);
		void commit_run();
		void commit_line();

		void operator =(const manipulator &rhs);

	private:
		layout::state _state;

		glyph_run *_current_run;
		text_line *_current_line;
		real_t _implicit_height;

		positioned_glyphs_container_t &_glyphs;
		glyph_runs_container_t &_glyph_runs;
		layout::text_lines_container_t &_text_lines;

	private:
		friend class layout;
	};



	template <typename LimitProcessorT, typename FontFactoryT>
	inline void layout::process(const richtext_t &text, LimitProcessorT limit_processor, FontFactoryT &font_factory_)
	{
		_glyphs.resize(static_cast<count_t>(text.size()));
		_box = zero();

		manipulator m(_glyphs, _glyph_runs, _text_lines);

		for (richtext_t::const_iterator range = text.ranges_begin(), ranges_end = text.ranges_end(); range != ranges_end;
			++range)
		{
			const font::ptr font_ = font_factory_.create_font(range->get_annotation().basic);

			m.begin_style(font_);
			limit_processor.begin_style(font_);
			for (std::string::const_iterator i = range->begin(), end = range->end(); i != end; )
			{
				if (eat_lf(i))
				{
					m.break_current_line();
					limit_processor.new_line();
					continue;
				}

				std::string::const_iterator next = i;
				const glyph &g = *font_->get_glyph_for_codepoint(utf8::next(next, end));

				if (!limit_processor.add_glyph(m, g.index, g.metrics.advance_x, i, next, end))
				{
					_text_lines.clear();
					return;
				}
			}
		}
		m.commit_line();
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
