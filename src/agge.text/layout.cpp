#include <agge.text/layout.h>

#include <agge/config.h>

using namespace std;

namespace agge
{
	namespace
	{
		template <typename VectorT>
		inline typename VectorT::value_type &duplicate_last(VectorT &container)
		{
			container.reserve(container.size() + 1);
			return *container.insert(container.end(), container.back());
		}
	}


	bool layout::state::operator !() const
	{	return !next;	}

	bool layout::state::operator <(const state &rhs) const
	{	return next < rhs.next;	}


	layout::manipulator::manipulator(positioned_glyphs_container_t &glyphs, glyph_runs_container_t &glyph_runs,
		layout::text_lines_container_t &text_lines)
		: _state(zero()), _glyphs(glyphs), _glyph_runs(glyph_runs), _text_lines(text_lines)
	{
		_glyph_runs.clear();
		_current_run = &*_glyph_runs.insert(_glyph_runs.end(), glyph_run(_glyphs));
		_text_lines.clear();
		_current_line = &*_text_lines.insert(_text_lines.end(), text_line(_glyph_runs));
		_state.runs_size = 1;
	}

	void layout::manipulator::begin_style(const font::ptr &font_)
	{
		const font_metrics m = font_->get_metrics();

		commit_run();
		_current_run->font_ = font_;
		_implicit_height = m.ascent + m.descent + m.leading;
	}

	void layout::manipulator::append_glyph(glyph_index_t index, real_t advance)
	{
		positioned_glyph &pg = _glyphs[static_cast<count_t>(_state.next++)];

		pg.index = index;
		pg.d.dx = advance, pg.d.dy = real_t();
		_state.extent += advance;
	}

	void layout::manipulator::trim_current_line(const layout::state &at)
	{
		_glyph_runs.erase(_glyph_runs.begin() + at.runs_size, _glyph_runs.end());
		_current_run = &_glyph_runs.back();
		_current_run->end_index = at.next;
		_state = at;
	}

	bool layout::manipulator::break_current_line()
	{
		commit_run();
		bool was_empty = _current_line->empty();
		commit_line();
		_current_run->offset = zero();
		return !was_empty;
	}

	void layout::manipulator::break_current_line(const layout::state &at, const layout::state &resume_at)
	{
		layout::state store = _state;

		_state = at;
		break_current_line();
		_state = store, _state.extent -= resume_at.extent, _state.runs_size = _glyph_runs.size();
		_current_run->end_index = _current_run->begin_index = resume_at.next;
	}

	const layout::state &layout::manipulator::get_state() const
	{	return _state;	}

	void layout::manipulator::commit_run()
	{
		if (_current_run->begin_index < _state.next)
		{
			_current_run->end_index = _state.next;
			_current_line->extend_end();
			_current_run = &duplicate_last(_glyph_runs);
			_current_run->begin_index = _state.next;
			_state.runs_size = _glyph_runs.size();
		}
		_current_run->offset = create_vector(_state.extent, real_t());
	}

	void layout::manipulator::commit_line()
	{
		commit_run();
		if (_current_line->empty())
		{
			_current_line->extent = _state.extent = real_t();
			_current_line->offset += create_vector(real_t(), _implicit_height);
		}
		else
		{
			real_t descent = real_t();
			pair<real_t, real_t> voffset;

			for (text_line::const_iterator i = _current_line->begin(), end = _current_line->end(); i != end; ++i)
			{
				const font_metrics grm = i->font_->get_metrics();

				if (voffset.first < grm.ascent)
					voffset.first = grm.ascent;
				if (voffset.second < grm.descent + grm.leading)
					voffset.second = grm.descent + grm.leading;
				if (descent < grm.descent)
					descent = grm.descent;
			}

			_current_line->offset += create_vector(real_t(), voffset.first);
			_current_line->extent = _state.extent;
			_current_line->descent = descent;
			_current_line = &duplicate_last(_text_lines);
			_current_line->begin_index = _current_line->end_index;
			_current_line->offset += create_vector(real_t(), voffset.second);
			_current_line->extent = _state.extent = real_t();
		}
	}
}
