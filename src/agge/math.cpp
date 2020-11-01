#include <agge/math.h>

#include <math.h>

namespace agge
{
	template <>
	inline float limits<float>::resolution()
	{	return 1e-6f;	}

	template <>
	inline double limits<double>::resolution()
	{	return 1e-15;	}


	const real_t c_qarc_bezier_k = real_t(0.551915024494);
	const real_t distance_epsilon = limits<real_t>::resolution();
	const real_t pi = real_t(3.14159265359);


	float sqrt(float x)
	{	return ::sqrtf(x);	}

	double sqrt(double x)
	{	return ::sqrt(x);	}

	float sin(float a)
	{	return ::sinf(a);	}

	float cos(float a)
	{	return ::cosf(a);}

	float acos(float v)
	{	return ::acosf(v);	}

	real_t optimal_circle_stepping(real_t radius, real_t scale)
	{
		int n = static_cast<int>(2.0f * pi / acos(radius / (radius + 0.125f / scale)));

		return 1.0f / static_cast<real_t>((n + 3) & ~3);
	}
}
