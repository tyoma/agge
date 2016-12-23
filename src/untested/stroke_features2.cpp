#include <agge/stroke_features.h>

#include <math.h>

namespace agge
{
	namespace
	{
		const real_t pi = static_cast<real_t>(3.14159265358979323846);

		float cos(float x)
		{	return ::cosf(x);	}

		double cos(double x)
		{	return ::cos(x);	}

		float sin(float x)
		{	return ::sinf(x);	}

		double sin(double x)
		{	return ::sin(x);	}

		float acos(float x)
		{	return ::acosf(x);	}

		double acos(double x)
		{	return ::acos(x);	}

		float atan2(float y, float x)
		{	return ::atan2f(y, x);	}

		double atan2(double y, double x)
		{	return ::atan2(y, x);	}
	}

	namespace caps
	{
		round::round(real_t approximation_scale)
			: _approximation_scale(approximation_scale)
		{	}

		void round::calc(points &output, real_t w, const point_r &v0, real_t d, const point_r &v1) const
		{
			d = w / d;

			const real_t dx = (v1.y - v0.y) * d;
			const real_t dy = (v1.x - v0.x) * d;
         const int n = static_cast<int>(pi / acos(w / (w + 0.125f / _approximation_scale)) * 2.0f);
         const real_t da = pi / (n + 1);
         real_t a = atan2(dy, -dx);

         output.push_back(create_point(v0.x - dx, v0.y + dy));
         for (int i = 0; i < n; i++)
         {
	         a += da;
            output.push_back(create_point(v0.x + cos(a) * w, v0.y + sin(a) * w));
         }
         output.push_back(create_point(v0.x + dx, v0.y - dy));
		}
	}
}
