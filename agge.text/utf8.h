#pragma once

#include <agge/config.h>

namespace agge
{
	struct utf8
	{
		typedef unsigned int codepoint;

		template <typename CharIteratorT>
		static codepoint next(CharIteratorT &iterator, CharIteratorT end, codepoint invalid = '?');

	private:
		template <typename CharIteratorT>
		static codepoint next_slow(CharIteratorT &iterator, CharIteratorT end, codepoint c, codepoint invalid);
	};



	template <typename CharIteratorT>
	AGGE_INLINE utf8::codepoint utf8::next(CharIteratorT &iterator, CharIteratorT end, codepoint invalid)
	{
		codepoint c = static_cast<unsigned char>(*iterator++);

		if (c < 0x80)
			return c;
		return next_slow(iterator, end, c, invalid);
	}

	template <typename CharIteratorT>
	inline utf8::codepoint utf8::next_slow(CharIteratorT &iterator, CharIteratorT end, codepoint c, codepoint invalid)
	{
		typedef unsigned char uchar;

		uchar remainder;

		if (c < 0xC0)
			return invalid;
		else if (c < 0xE0)
			remainder = 1, c &= 0x1F;
		else if (c < 0xF0)
			remainder = 2, c &= 0x0F;
		else if (c < 0xF8)
			remainder = 3, c &= 0x07;
		else
			return invalid;

		if (remainder > end - iterator)
			return iterator = end, invalid;

		while (remainder--)
		{
			const uchar continuation = static_cast<uchar>(*iterator++);

			if ((continuation < 0x80) | (0xC0 <= continuation))
				return iterator += remainder, invalid;
			c <<= 6;
			c += continuation & 0x3F;
		}
		return c;
	}
}
