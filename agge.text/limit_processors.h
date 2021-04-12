#pragma once

#include "layout_primitives.h"
#include "tools.h"

namespace agge
{
	class wrap_processor
	{
	public:
		wrap_processor(real_t limit)
			: _limit(limit)
		{	reset();	}

		real_t init_newline(glyph_run &accumulator)
		{
			real_t occupied = real_t();

			if (_carry)
			{
				accumulator.begin_index = _sow_index;
				occupied = _sow_width;
			}
			reset();
			return occupied;
		}

		template <typename CharIteratorT>
		bool accept_glyph(real_t advance, CharIteratorT &i, CharIteratorT text_end, size_t &end_index, real_t &occupied)
		{
			const bool space = is_space(*i);

			if (_previous_space != space)
			{
				if (space)
					_eow_index = end_index, _eow_width = occupied;
				else
					_sow_index = end_index, _sow_width = occupied;
				_previous_space = space;
			}
			if (occupied + advance > _limit)
			{
				if (!_eow_index)
					return false;	// Next line - emergency mid-word break

				// Next line - normal word-boundary break
				real_t sow_width = occupied - _sow_width;

				end_index = _eow_index;
				occupied = _eow_width;
				if (_sow_index > _eow_index)
				{
					// New word was actually found after the last matched end-of-word.
					_sow_width = sow_width;
					_carry = true;
				}
				else
				{
					// No new word found before - let's scan for it ourselves.
					eat_spaces(i, text_end);
				}
				return false;
			}
			return true;
		}

	private:
		void reset()
		{
			_carry = false;
			_eow_index = _sow_index = size_t();
			_eow_width = _sow_width = real_t();
			_previous_space = false;
		}

		const wrap_processor &operator =(const wrap_processor &rhs);

	private:
		const real_t _limit;
		size_t _eow_index, _sow_index;
		real_t _eow_width, _sow_width;
		bool _previous_space, _carry;
	};
}
