#pragma once

#include "tools.h"

namespace agge
{
	class precise_delta
	{
	public:
		precise_delta(int numerator, int denominator)
			: _acc(0)
		{
			const float q = static_cast<float>(numerator) / denominator;
			const int iq = reinterpret_cast<const int &>(q);
			int exp = (((iq >> 23) & 0xFF) - 127) - 0x15;
			int m = iq & 0x7FFFFF | 0x800000;
			int exp_fractional = -negative_or_zero(exp);

			m <<= positive_or_zero(exp);
			_exp = negative_or_zero(exp_fractional - 0x1E) + 0x1E;
			m >>= 2 + exp_fractional - _exp;
			_quotient = (m ^ (iq >> 31)) + (static_cast<unsigned>(iq) >> 31);
		}

		void multiply(int k)
		{
			_delta_fraction = k * _quotient;
		}

		int get() const
		{
			return _delta;
		}

		void next()
		{
			_acc += _delta_fraction;
			_delta = _acc >> _exp;
			_acc -= _delta << _exp;
		}

	private:
		static int positive_or_zero(int value)
		{
			return (static_cast<int>(value ^ 0x80000000) >> 31) & value;
		}

		static int negative_or_zero(int value)
		{
			return (value >> 31) & value;
		}

	private:
		int _acc;
		int _quotient;
		int _delta_fraction;
		int _delta;
		unsigned int _exp;
	};
}
