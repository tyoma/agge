#include <agge.text/layout_builder.h>

#include <agge/math.h>
#include <agge/minmax.h>

namespace agge
{
	namespace
	{
		template <typename VectorT, typename T>
		typename VectorT::value_type &duplicate_last(VectorT &container, T pivot)
		{
			container.reserve(container.size() + 1);
			container.back().end_index = pivot;
			typename VectorT::value_type &entry = *container.insert(container.end(), container.back());
			entry.begin_index = pivot;
			return entry;
		}
	}


	bool layout_builder::state::operator !() const
	{	return !next;	}

	bool layout_builder::state::operator <(const state &rhs) const
	{	return next < rhs.next;	}


	layout_builder::layout_builder(positioned_glyphs_container_t &glyphs, glyph_runs_container_t &glyph_runs,
		text_lines_container_t &text_lines)
		: _state(zero()), _glyphs(glyphs), _glyph_runs(glyph_runs), _text_lines(text_lines)
	{
		_glyph_runs.clear();
		_current_run = &*_glyph_runs.insert(_glyph_runs.end(), glyph_run(_glyphs));
		_text_lines.clear();
		_current_line = &*_text_lines.insert(_text_lines.end(), text_line(_glyph_runs));
	}

	void layout_builder::begin_style(const font::ptr &font_)
	{
		const font_metrics m = font_->get_metrics();

		if (_current_run->begin_index < _state.next)
			_current_run = &duplicate_last(_glyph_runs, _state.next);
		_current_run->font_ = font_;
		_current_run->offset = create_vector(_state.extent, real_t());
		_implicit_height = m.ascent + m.descent + m.leading;
	}

	void layout_builder::trim_current_line(const state &at)
	{
		while (at.next < _current_run->begin_index)
		{
			_glyph_runs.pop_back();
			_current_run = &_glyph_runs.back();
		}
		_current_run->end_index = at.next;
		_state = at;
	}

	bool layout_builder::break_current_line()
	{
		if (_current_run->begin_index < _state.next)
			_current_run = &duplicate_last(_glyph_runs, _state.next);
		_current_run->offset = zero();
		return commit_line();
	}

	void layout_builder::break_current_line(const state &at, const state &resume_at)
	{
		state store = _state;

		_state = at;
		break_current_line();
		_state = store, _state.extent -= resume_at.extent;
		_current_run->end_index = _current_run->begin_index = resume_at.next;
	}

	bool layout_builder::commit_line()
	{
		const size_t end_index = _glyph_runs.size() - 1;

		if (_current_line->begin_index == end_index)
		{
			_current_line->offset += create_vector(real_t(), _implicit_height);
			_state.extent = _current_line->extent = real_t();
			return false;
		}
		else
		{
			real_t ascent = real_t(), descent = real_t(), descent_leading = real_t();

			for (size_t i = _current_line->begin_index; i != end_index; ++i)
			{
				const font_metrics grm = _glyph_runs[i].font_->get_metrics();

				update_max(ascent, grm.ascent);
				update_max(descent_leading, grm.descent + grm.leading);
				update_max(descent, grm.descent);
			}

			_current_line->offset += create_vector(real_t(), ascent);
			_current_line->extent = _state.extent;
			_current_line->descent = descent;
			_current_line = &duplicate_last(_text_lines, end_index);
			_current_line->offset += create_vector(real_t(), descent_leading);
			_state.extent = _current_line->extent = real_t();
			return true;
		}
	}
}
