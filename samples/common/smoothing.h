#pragma once

template <typename T>
class smoothing
{
public:
	smoothing()
		: _value({})
	{	}

	void add(const T &new_value)
	{	_value = 0.99 * _value + 0.01 * new_value;	}

	const T &get() const
	{	return _value;	}

private:
	T _value;
	double _alpha;
};
