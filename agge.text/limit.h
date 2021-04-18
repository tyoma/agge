#pragma once

#include "layout_builder.h"

#include <agge/config.h>

namespace agge
{
	namespace limit
	{
		struct base
		{
			void begin_style(const layout_builder &builder);
			void new_line();
			template <typename CharIteratorT>
			bool add_glyph(layout_builder &builder, glyph_index_t glyph_index, real_t advance,
				CharIteratorT &i, CharIteratorT next, CharIteratorT end);
		};

		typedef base none;

		class wrap : public base
		{
		public:
			wrap(real_t limit);

			void new_line();

			template <typename CharIteratorT>
			bool add_glyph(layout_builder &builder, glyph_index_t glyph_index, real_t advance,
				CharIteratorT &i, CharIteratorT next, CharIteratorT end);

		private:
			const wrap &operator =(const wrap &rhs);

			template <typename CharIteratorT>
			bool break_current_line(layout_builder &builder, CharIteratorT &i, CharIteratorT end);

		private:
			const real_t _limit;
			layout_builder::state _eow, _sow;
			bool _previous_space;
		};

		class ellipsis : public base
		{
		public:
			ellipsis(real_t limit, codepoint_t symbol = 0x2026 /*h-ellipsis*/);
		};



		inline void base::begin_style(const layout_builder &/*builder*/)
		{	}

		inline void base::new_line()
		{	}

		template <typename CharIteratorT>
		inline bool base::add_glyph(layout_builder &builder, glyph_index_t glyph_index, real_t advance,
			CharIteratorT &i, CharIteratorT next, CharIteratorT /*end*/)
		{
			builder.append_glyph(glyph_index, advance);
			i = next;
			return true;
		}


		inline wrap::wrap(real_t limit)
			: _limit(limit), _sow(zero())
		{	new_line();	}

		template <typename CharIteratorT>
		inline bool wrap::add_glyph(layout_builder &builder, glyph_index_t glyph_index, real_t advance,
			CharIteratorT &i, CharIteratorT next, CharIteratorT end)
		{
			const bool space = is_space(*i);
			const layout_builder::state &state = builder.get_state();

			if (_previous_space != space)
				(space ? _eow : _sow) = state, _previous_space = space;
			return state.extent + advance > _limit ? break_current_line(builder, i, end)
				: base::add_glyph(builder, glyph_index, advance, i, next, end);
		}

		inline void wrap::new_line()
		{
			_eow = zero();
			_previous_space = false; // TODO: do we need to set it to 'true' (which is logically correct)?
		}

		template <typename CharIteratorT>
		AGGE_AVOID_INLINE inline bool wrap::break_current_line(layout_builder &builder, CharIteratorT &i, CharIteratorT end)
		{
			if (!_eow)
			{
				// Emergency mid-word break.
				return builder.break_current_line();
			}
			else if (_eow < _sow)
			{
				// New word was actually found after the last matched end-of-word.
				builder.break_current_line(_eow, _sow);
			}
			else
			{
				// No new word found before - let's scan for it ourselves.
				builder.trim_current_line(_eow);
				builder.break_current_line();
				eat_spaces(i, end);
			}
			new_line();
			return true;
		}
	}
}
