#pragma once

#include "layout.h"

namespace agge
{
	namespace limit
	{
		struct base
		{
			void begin_style(const font::ptr &font_);
			void new_line();
			template <typename CharIteratorT>
			bool add_glyph(layout::manipulator &manipulator, glyph_index_t glyph_index, real_t advance,
				CharIteratorT &i, CharIteratorT next, CharIteratorT end);
		};

		typedef base unlimited;

		class wrap : public base
		{
		public:
			wrap(real_t limit);

			void new_line();

			template <typename CharIteratorT>
			bool add_glyph(layout::manipulator &manipulator, glyph_index_t glyph_index, real_t advance,
				CharIteratorT &i, CharIteratorT next, CharIteratorT end);

		private:
			const wrap &operator =(const wrap &rhs);

			template <typename CharIteratorT>
			bool break_current_line(layout::manipulator &manipulator, CharIteratorT &i, CharIteratorT end);

		private:
			const real_t _limit;
			layout::state _eow, _sow;
			bool _previous_space;
		};



		inline void base::begin_style(const font::ptr &/*font_*/)
		{	}

		inline void base::new_line()
		{	}

		template <typename CharIteratorT>
		inline bool base::add_glyph(layout::manipulator &manipulator, glyph_index_t glyph_index, real_t advance,
			CharIteratorT &i, CharIteratorT next, CharIteratorT /*end*/)
		{
			manipulator.append_glyph(glyph_index, advance);
			i = next;
			return true;
		}


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
				(space ? _eow : _sow) = state, _previous_space = space;
			return state.extent + advance > _limit ? break_current_line(manipulator, i, end)
				: base::add_glyph(manipulator, glyph_index, advance, i, next, end);
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
				manipulator.trim_current_line(_eow);
				manipulator.break_current_line();
				eat_spaces(i, end);
			}
			new_line();
			return true;
		}
	}
}
