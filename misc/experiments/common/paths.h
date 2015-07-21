#pragma once

#include <aggx/agg_math.h>
#include <aggx/basics.h>

#include <vector>

namespace demo
{
	typedef std::vector< std::pair<std::pair<aggx::real, aggx::real>, unsigned> > AggPath;

	class agg_path_adaptor
	{
	public:
		agg_path_adaptor(const AggPath &path)
			: _path(path), _position(_path.begin())
		{
		}

		void rewind(unsigned)
		{
			_position = _path.begin();
		}

		unsigned vertex(float* x, float* y)
		{
			if (_position == _path.end())
				return aggx::path_cmd_stop;
			else
				return *x = _position->first.first, *y = _position->first.second, _position++->second;
		}

		unsigned vertex(double* x, double* y)
		{
			if (_position == _path.end())
				return aggx::path_cmd_stop;
			else
				return *x = _position->first.first, *y = _position->first.second, _position++->second;
		}

	private:
		const AggPath &_path;
		AggPath::const_iterator _position;
	};

	void pathStart(AggPath &/*path*/)
	{	}

	void pathMoveTo(AggPath &path, aggx::real x, aggx::real y)
	{	path.push_back(std::make_pair(std::make_pair(x, y), aggx::path_cmd_move_to));	}
	
	void pathLineTo(AggPath &path, aggx::real x, aggx::real y)
	{	path.push_back(std::make_pair(std::make_pair(x, y), aggx::path_cmd_line_to));	}

	void pathEnd(AggPath &path)
	{	path.push_back(std::make_pair(std::make_pair(0.0f, 0.0f), aggx::path_cmd_stop));	}

	template <typename TargetT, typename PathT>
	void flatten(TargetT &destination, PathT &source)
	{
		unsigned cmd;
		aggx::real x, y;

		source.rewind(0);
		while (!is_stop(cmd = source.vertex(&x, &y)))
			destination.push_back(std::make_pair(std::make_pair(x, y), cmd));
	}

	template <typename TargetT>
	void spiral(TargetT &target, aggx::real x, aggx::real y, aggx::real r1, aggx::real r2, aggx::real step, aggx::real start_angle)
	{
		const aggx::real k = 4.0f;

		bool start = true;

		pathStart(target);
		for (aggx::real angle = start_angle, dr = k * step / 45.0f, da = k / 180.0f * aggx::pi; r1 < r2; r1 += dr, angle += da, start = false)
		{
			const aggx::real px = x + aggx::cos(angle) * r1, py = y + aggx::sin(angle) * r1;

			if (start)
				pathMoveTo(target, px, py);
			else
				pathLineTo(target, px, py);
		}
		pathEnd(target);
	}

}
