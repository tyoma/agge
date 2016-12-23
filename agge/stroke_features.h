#pragma once

#include "stroke.h"

namespace agge
{
	namespace caps
	{
		class butt : public stroke::cap
		{
		public:
			virtual void calc(points &output, real_t w, const point_r &v0, real_t d, const point_r &v1) const;
		};

		class round : public stroke::cap
		{
		public:
			explicit round(real_t approximation_scale = 1.0f);

			virtual void calc(points &output, real_t w, const point_r &v0, real_t d, const point_r &v1) const;

		private:
			real_t _approximation_scale;
		};
	}

	namespace joins
	{
		class bevel : public stroke::join
		{
		public:
			virtual void calc(points &output, real_t w, const point_r &v0, real_t d01, const point_r &v1, real_t d12, const point_r &v2) const;
		};


		class miter : public stroke::join
		{
		public:
			explicit miter(real_t limit);

			virtual void calc(points &output, real_t w, const point_r &v0, real_t d01, const point_r &v1, real_t d12, const point_r &v2) const;

		private:
			real_t _limit;
		};
	}
}
