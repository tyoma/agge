#pragma once

#include "layout_builder.h"

#include <agge/config.h>

namespace agge
{
	namespace limit
	{
		struct base
		{
			template <typename BuilderT>
			void begin_style(const BuilderT &builder);
			void new_line();
			template <typename BuilderT, typename CharIteratorT>
			bool add_glyph(BuilderT&builder, glyph_index_t glyph_index, real_t extent,
				CharIteratorT &i, CharIteratorT next, CharIteratorT end);
		};

		typedef base none;

		class wrap : public base
		{
		public:
			wrap(real_t limit);

			void new_line();

			template <typename BuilderT, typename CharIteratorT>
			bool add_glyph(BuilderT &builder, glyph_index_t glyph_index, real_t extent,
				CharIteratorT &i, CharIteratorT next, CharIteratorT end);

		private:
			const wrap &operator =(const wrap &rhs);

			template <typename BuilderT, typename CharIteratorT>
			bool break_current_line(BuilderT &builder, CharIteratorT &i, CharIteratorT end);

		private:
			const real_t _limit;
			layout_builder::state _eow, _sow;
			bool _previous_space;
		};

		class ellipsis : public base
		{
		public:
			ellipsis(real_t limit, codepoint_t symbol = 0x2026 /* Unicode for 'Horizontal Ellipsis' */);

			template <typename BuilderT>
			void begin_style(const BuilderT &builder);

			void new_line();

			template <typename BuilderT, typename CharIteratorT>
			bool add_glyph(BuilderT &builder, glyph_index_t glyph_index, real_t extent,
				CharIteratorT &i, CharIteratorT next, CharIteratorT end);

		private:
			template <typename BuilderT, typename CharIteratorT>
			bool trim(BuilderT &builder, CharIteratorT &i, CharIteratorT next);

		private:
			real_t _limit;
			layout_builder::state _at;
			codepoint_t _symbol_codepoint;
			std::pair<glyph_index_t, real_t> _symbol, _at_symbol;
			bool _stop;
		};



		template <typename BuilderT>
		inline void base::begin_style(const BuilderT &/*builder*/)
		{	}

		inline void base::new_line()
		{	}

		template <typename BuilderT, typename CharIteratorT>
		inline bool base::add_glyph(BuilderT &builder, glyph_index_t glyph_index, real_t extent,
			CharIteratorT &i, CharIteratorT next, CharIteratorT /*end*/)
		{
			builder.append_glyph(glyph_index, extent);
			i = next;
			return true;
		}


		inline wrap::wrap(real_t limit)
			: _limit(limit), _sow(zero())
		{	new_line();	}

		template <typename BuilderT, typename CharIteratorT>
		inline bool wrap::add_glyph(BuilderT &builder, glyph_index_t glyph_index, real_t extent,
			CharIteratorT &i, CharIteratorT next, CharIteratorT end)
		{
			const bool space = is_space(*i);
			const layout_builder::state &state = builder.get_state();

			if (_previous_space != space)
				(space ? _eow : _sow) = state, _previous_space = space;
			return state.extent + extent > _limit ? break_current_line(builder, i, end)
				: base::add_glyph(builder, glyph_index, extent, i, next, end);
		}

		inline void wrap::new_line()
		{
			_eow = zero();
			_previous_space = false; // TODO: do we need to set it to 'true' (which is logically correct)?
		}

		template <typename BuilderT, typename CharIteratorT>
		AGGE_AVOID_INLINE inline bool wrap::break_current_line(BuilderT &builder, CharIteratorT &i, CharIteratorT end)
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


		inline ellipsis::ellipsis(real_t limit, codepoint_t symbol)
			: _limit(limit), _symbol_codepoint(symbol)
		{	new_line();	}

		template <typename BuilderT>
		inline void ellipsis::begin_style(const BuilderT &builder)
		{	_symbol = builder.current_glyph(_symbol_codepoint);	}

		inline void ellipsis::new_line()
		{	_at = zero(), _stop = false;	}

		template <typename BuilderT, typename CharIteratorT>
		inline bool ellipsis::add_glyph(BuilderT &builder, glyph_index_t glyph_index, real_t extent,
			CharIteratorT &i, CharIteratorT next, CharIteratorT end)
		{
			if (_stop)
				return i = next, true;

			const layout_builder::state &state = builder.get_state();
			const real_t candidate_position = state.extent + _symbol.second;

			// If the glyph being added would prevent addition, store this state to trim to.
			if ((candidate_position <= _limit) & (candidate_position + extent > _limit))
				_at = state, _at_symbol = _symbol;

			// If the glyph being added does not overlap the limit - add it.
			if (state.extent + extent < /*TODO: <=*/ _limit)
				return base::add_glyph(builder, glyph_index, extent, i, next, end);

			// If no ellipsis position is latched - we cannot gracefully trim, so just halt the processing.
			return !_at ? false : trim(builder, i, next);
		}

		template <typename BuilderT, typename CharIteratorT>
		AGGE_AVOID_INLINE inline bool ellipsis::trim(BuilderT &builder, CharIteratorT &i, CharIteratorT next)
		{
			builder.trim_current_line(_at);
			builder.append_glyph(_at_symbol.first, _at_symbol.second);
			_stop = true;
			i = next;
			return true;
		}
	}
}
