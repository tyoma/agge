#include <agge/stroke_features.h>

#include <agge/curves.h>
#include <agge/math.h>
#include <agge/tools.h>

namespace agge
{
	namespace caps
	{
		void butt::calc(points &output, real_t hw, const point_r &v0, real_t d, const point_r &v1) const
		{
			d = hw / d;

			const real_t dx = (v1.x - v0.x) * d;
			const real_t dy = (v1.y - v0.y) * d;

			output.push_back(create_point(v0.x - dy, v0.y + dx));
			output.push_back(create_point(v0.x + dy, v0.y - dx));
		}


		triangle::triangle(real_t tip_extension)
			: _tip_extension(tip_extension)
		{	}

		void triangle::calc(points &output, real_t hw, const point_r &v0, real_t d, const point_r &v1) const
		{
			d = hw / d;

			const real_t dx = (v1.x - v0.x) * d;
			const real_t dy = (v1.y - v0.y) * d;

			output.push_back(create_point(v0.x - dy, v0.y + dx));
			output.push_back(create_point(v0.x - dx * _tip_extension, v0.y - dy * _tip_extension));
			output.push_back(create_point(v0.x + dy, v0.y - dx));
		}


		void round::calc(points &output, real_t hw, const point_r &v0, real_t d, const point_r &v1) const
		{
			d = hw / d;

			const real_t step = 4.0f * optimal_circle_stepping(hw);
			const real_t dx = (v1.x - v0.x) * d, d2x = c_qarc_bezier_k * dx;
			const real_t dy = (v1.y - v0.y) * d, d2y = c_qarc_bezier_k * dy;

			real_t xb = v0.x - dy, yb = v0.y + dx;

			output.push_back(create_point(xb, yb));

			real_t xc1 = xb - d2x, yc1 = yb - d2y;
			real_t xe = v0.x - dx, ye = v0.y - dy;
			real_t xc2 = xe - d2y, yc2 = ye + d2x;

			for (real_t t = step; t < 1.0f; t += step)
			{
				point_r p;

				cbezier::calculate(&p.x, &p.y, xb, yb, xc1, yc1, xc2, yc2, xe, ye, t);
				output.push_back(p);
			}
			output.push_back(create_point(xe, ye));

			xb = xe, yb = ye;
			xc1 = xb + d2y, yc1 = yb - d2x;
			xe = v0.x + dy, ye = v0.y - dx;
			xc2 = xe - d2x, yc2 = ye - d2y;

			for (real_t t = step; t < 1.0f; t += step)
			{
				point_r p;

				cbezier::calculate(&p.x, &p.y, xb, yb, xc1, yc1, xc2, yc2, xe, ye, t);
				output.push_back(p);
			}
			output.push_back(create_point(xe, ye));
		}
	}

	namespace joins
	{
		void bevel::calc(points &output, real_t hw, const point_r &v0, real_t d01, const point_r &v1, real_t d12, const point_r &v2) const
		{
			d01 = hw / d01;
			d12 = hw / d12;
				
			const real_t dx1 = d01 * (v1.x - v0.x);
			const real_t dy1 = d01 * (v1.y - v0.y);
			const real_t dx2 = d12 * (v2.x - v1.x);
			const real_t dy2 = d12 * (v2.y - v1.y);

			output.push_back(create_point(v1.x + dy1, v1.y - dx1));
			output.push_back(create_point(v1.x + dy2, v1.y - dx2));
		}
	}
}
