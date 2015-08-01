#pragma once

#include <windows.h>

inline double counter_period()
{
	LARGE_INTEGER frequency;

	::QueryPerformanceFrequency(&frequency);
	return 1000.0 / frequency.QuadPart;
}

const double c_counter_period = counter_period();

inline double stopwatch(LARGE_INTEGER &counter)
{
	double value;
	LARGE_INTEGER current;

	::QueryPerformanceCounter(&current);
	value = c_counter_period * (current.QuadPart - counter.QuadPart);
	counter = current;
	return value;
}
