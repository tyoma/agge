#include <agge.text/layout.h>

#include <agge/config.h>
#include <agge/math.h>
#include <agge/tools.h>
#include <agge.text/font_factory.h>
#include <agge.text/limit_processors.h>

using namespace std;

namespace agge
{
	namespace
	{
		template <typename VectorT>
		typename VectorT::value_type &duplicate_last(VectorT &container)
		{	return container.reserve(container.size() + 1), *container.insert(container.end(), container.back());	}
	}

	pair<real_t, real_t> layout::setup_line_metrics(text_line &line)
	{
		real_t descent = real_t();
		pair<real_t, real_t> m((real_t()), (real_t()));

		for (text_line::const_iterator i = line.begin(), end = line.end(); i != end; ++i)
		{
			const font_metrics grm = i->font_->get_metrics();

			if (m.first < grm.ascent)
				m.first = grm.ascent;
			if (m.second < grm.descent + grm.leading)
				m.second = grm.descent + grm.leading;
			if (descent < grm.descent)
				descent = grm.descent;
		}
		line.descent = descent;
		return m;
	}

	pair<real_t, real_t> layout::setup_line_metrics(text_line &line, const font_metrics &m)
	{	return line.empty() ? make_pair(m.ascent, m.descent + m.leading) : setup_line_metrics(line);	}

	layout::layout(font_factory &factory)
		: _factory(factory), _limit_width(1e30f)
	{	}

	void layout::process(const richtext_t &text)
	{
		_text_lines.clear();
		_glyph_runs.clear();
		_glyphs.clear();
		_box = zero();

		text_line *current_line = &*_text_lines.insert(_text_lines.end(), text_line(_glyph_runs));
		glyph_run *current = &*_glyph_runs.insert(_glyph_runs.end(), glyph_run(_glyphs));
		wrap_processor limit_processor;

		for (richtext_t::const_iterator range = text.ranges_begin(); range != text.ranges_end(); ++range)
		{
			current->font_ = _factory.create_font(range->get_annotation().basic);
			current->offset = create_vector(current_line->width, real_t());

			for (string::const_iterator i = range->begin(), end = range->end(), previous = i;
				populate_glyph_run(_glyphs, *current, limit_processor, _limit_width - current_line->width, i, end);
				previous = i)
			{
				if ((i == previous) & current_line->empty())
				{
					// Emergency: width limit is too small to layout even a single character - bailing out!
					_text_lines.clear();
					return;
				}
				commit_glyph_run(*current_line, current);
				limit_processor.init_newline(*current);

				const pair<real_t, real_t> m = setup_line_metrics(*current_line, current->font_->get_metrics());

				current_line->offset += create_vector(real_t(), m.first);
				if (!current_line->empty())
				{
					current_line = &duplicate_last(_text_lines);
					current_line->begin_index = current_line->end_index;
					_box.w = agge_max(_box.w, current_line->width);
					current_line->width = real_t();
				}
				current_line->offset += create_vector(real_t(), m.second);
			}

			// Commit any non-empty content of the current glyph run and prepare the next one.
			commit_glyph_run(*current_line, current);
		}
		if (current_line->empty())
			_text_lines.pop_back();
		else
			current_line->offset += create_vector(real_t(), setup_line_metrics(*current_line).first);
		if (!_text_lines.empty())
		{
			const text_line &last = _text_lines.back();

			_box.w = agge_max(_box.w, last.width);
			_box.h = last.offset.dy + last.descent;
		}
	}

	void layout::commit_glyph_run(text_line &current_line, glyph_run *&current)
	{
		if (!current->empty())
		{
			current_line.extend_end();
			current = &duplicate_last(_glyph_runs);
			current_line.width += current->width;
			current->width = real_t();
			current->set_end();
		}
	}
}
