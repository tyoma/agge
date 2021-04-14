#pragma once

#include "layout.h"

namespace agge
{
	namespace limit
	{
		struct unlimited
		{
			template <typename CharIteratorT>
			bool add_glyph(layout::manipulator &manipulator, glyph_index_t glyph_index, real_t advance,
				CharIteratorT &i, CharIteratorT next, CharIteratorT end);
			void new_line();
		};

		class wrap
		{
		public:
			wrap(real_t limit);

			template <typename CharIteratorT>
			bool add_glyph(layout::manipulator &manipulator, glyph_index_t glyph_index, real_t advance,
				CharIteratorT &i, CharIteratorT next, CharIteratorT end);
			void new_line();

		private:
			const wrap &operator =(const wrap &rhs);

			template <typename CharIteratorT>
			bool break_current_line(layout::manipulator &manipulator, CharIteratorT &i, CharIteratorT end);

		private:
			const real_t _limit;
			layout::state _eow, _sow;
			bool _previous_space;
		};



		template <typename CharIteratorT>
		inline bool unlimited::add_glyph(layout::manipulator &manipulator, glyph_index_t glyph_index, real_t advance,
			CharIteratorT &i, CharIteratorT next, CharIteratorT /*end*/)
		{
			manipulator.append_glyph(glyph_index, advance);
			i = next;
			return true;
		}

		inline void unlimited::new_line()
		{	}


		inline wrap::wrap(real_t limit)
			: _limit(limit), _sow(zero())
		{	new_line();	}

		template <typename CharIteratorT>
		inline bool wrap::add_glyph(layout::manipulator &manipulator, glyph_index_t glyph_index, real_t advance,
			CharIteratorT &i, CharIteratorT next, CharIteratorT end)
		{
			const bool space = is_space(*i);
			const layout::state &state = manipulator.get_state();

			if (_previous_space != space)
			{
				(space ? _eow : _sow) = state;
				_previous_space = space;
			}

			if (state.extent + advance > _limit)
				return break_current_line(manipulator, i, end);
			manipulator.append_glyph(glyph_index, advance);
			i = next;
			return true;
		}

		inline void wrap::new_line()
		{
			_eow = zero();
			_previous_space = false; // TODO: do we need to set it to 'true' (which is logically correct)?
		}

		template <typename CharIteratorT>
		AGGE_AVOID_INLINE inline bool wrap::break_current_line(layout::manipulator &manipulator, CharIteratorT &i, CharIteratorT end)
		{
			if (!_eow)
			{
				// Emergency mid-word break.
				return manipulator.break_current_line();
			}
			else if (_eow < _sow)
			{
				// New word was actually found after the last matched end-of-word.
				manipulator.break_current_line(_eow, _sow);
			}
			else
			{
				// No new word found before - let's scan for it ourselves.
				manipulator.break_current_line(_eow);
				eat_spaces(i, end);
			}
			new_line();
			return true;
		}
	}
}
