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
			const unsigned int iq = reinterpret_cast<const unsigned int &>(q);
			int m = (iq & 0x7FFFFF) | 0x800000;
			int exp = ((iq & 0x7F800000) >> 23) - 127;

			if (iq & 0x80000000)
				m = -m;

			exp = exp - 0x15;
			m <<= agge_max(0, exp);
			exp = agge_max(0, -exp);
			_exp = agge_min(0x1E, exp);
			_quotient = m >> (2 + exp - _exp);
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
		int _acc;
		int _quotient;
		int _delta_fraction;
		int _exp;
		int _delta;
	};
}
