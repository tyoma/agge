#pragma once

#include "layout_primitives.h"
#include "tools.h"

namespace agge
{
	namespace limit
	{
		struct unlimited
		{
			real_t init_newline(glyph_run &accumulator);

			template <typename CharIteratorT>
			bool accept_glyph(real_t advance, CharIteratorT &i, CharIteratorT text_end, size_t &end_index,
				real_t &extent);
		};

		class wrap
		{
		public:
			wrap(real_t limit);

			real_t init_newline(glyph_run &accumulator);

			template <typename CharIteratorT>
			bool accept_glyph(real_t advance, CharIteratorT &i, CharIteratorT text_end, size_t &end_index,
				real_t &extent);

		private:
			void reset();
			const wrap &operator =(const wrap &rhs);

		private:
			const real_t _limit;
			real_t _eow_width, _sow_width;
			size_t _eow_index, _sow_index;
			bool _previous_space : 1, _carry : 1;
		};



		inline real_t unlimited::init_newline(glyph_run &/*accumulator*/)
		{	return real_t();	}

		template <typename CharIteratorT>
		inline bool unlimited::accept_glyph(real_t /*advance*/, CharIteratorT &/*i*/, CharIteratorT /*text_end*/,
			size_t &/*end_index*/, real_t &/*extent*/)
		{	return true;	}


		inline wrap::wrap(real_t limit)
			: _limit(limit)
		{	reset();	}

		inline real_t wrap::init_newline(glyph_run &accumulator)
		{
			real_t extent = real_t();

			if (_carry)
			{
				accumulator.begin_index = _sow_index;
				extent = _sow_width;
			}
			reset();
			return extent;
		}

		template <typename CharIteratorT>
		inline bool wrap::accept_glyph(real_t advance, CharIteratorT &i, CharIteratorT text_end, size_t &end_index, real_t &extent)
		{
			const bool space = is_space(*i);

			if (_previous_space != space)
			{
				if (space)
					_eow_index = end_index, _eow_width = extent;
				else
					_sow_index = end_index, _sow_width = extent;
				_previous_space = space;
			}
			if (extent + advance > _limit)
			{
				if (!_eow_index)
					return false;	// Next line - emergency mid-word break

				// Next line - normal word-boundary break
				real_t sow_width = extent - _sow_width;

				end_index = _eow_index;
				extent = _eow_width;
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

		inline void wrap::reset()
		{
			_carry = false;
			_eow_index = _sow_index = size_t();
			_eow_width = _sow_width = real_t();
			_previous_space = false;
		}

	}
}
