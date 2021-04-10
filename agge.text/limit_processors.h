#pragma once

#include "layout_primitives.h"
#include "tools.h"

namespace agge
{
	class wrap_processor
	{
	public:
		wrap_processor(glyph_run &carry)
			: _carry(carry)
		{	}

		void on_new_run()
		{
			_carry.offset = zero()/*, _carry.width = real_t()*/;
			_eow_index = _sow_index = size_t();
			_eow_width = _sow_width = real_t();
			_previous_space = false;
		}

		void on_linefeed() const
		{	_carry.set_end();	}

		void analyze_character(char c, const glyph_run &accumulator)
		{
			const bool space = is_space(c);

			if (_previous_space == space)
				return;
			else if (space)
				_eow_index = accumulator.end_index, _eow_width = accumulator.width;
			else
				_sow_index = accumulator.end_index, _sow_width = accumulator.width;
			_previous_space = space;
		}

		template <typename CharIteratorT>
		void on_limit_reached(CharIteratorT &i, CharIteratorT text_end, glyph_run &accumulator) const
		{
			_carry.set_end();
			if (!_eow_index)
				return;	// Next line - emergency mid-word break

			// Next line - normal word-boundary break
			real_t sow_width = accumulator.width - _sow_width;

			accumulator.end_index = _eow_index;
			accumulator.width = _eow_width;
			if (_sow_index > _eow_index)
			{
				// New word was actually found after the last matched end-of-word.
				_carry.begin_index = _sow_index;
				_carry.width = sow_width;
			}
			else
			{
				// No new word found before - let's scan for it ourselves.
				eat_spaces(i, text_end);
			}
		}

	private:
		const wrap_processor &operator =(const wrap_processor &rhs);

	private:
		glyph_run &_carry;
		size_t _eow_index, _sow_index;
		real_t _eow_width, _sow_width;
		bool _previous_space;
	};
}
