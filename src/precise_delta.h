#pragma once

namespace agge
{
	class precise_delta
	{
	public:
		precise_delta(int numerator, int denominator)
			: _quotient((numerator << _ep_shift) / denominator), _acc(0)
		{	}

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
			_delta = _acc >> _ep_shift;
			_acc -= _delta << _ep_shift;
		}

	private:
		enum { _ep_shift = 14 };

	private:
		int _delta;
		int _quotient;
		int _delta_fraction;
		int _acc;
	};
}
