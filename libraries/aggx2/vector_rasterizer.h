#pragma once

#include <utility>
#include <vector>

namespace aggx
{
	class vector_rasterizer
	{
	public:
		enum
		{
			extra_precision_shift = 8,
			_1_shift = 8,

			extra_precision = 1 << extra_precision_shift,
			_1 = 1 << _1_shift,
			_1_mask = _1 - 1,
		};

#pragma pack(push, 1)
		struct cell
		{
			short x, y;
			int area, cover;

			static const cell empty;
		};
#pragma pack(pop)

		typedef std::vector<cell> cells_container;
		typedef std::pair<int, int> range;

	public:
		explicit vector_rasterizer(cells_container &cells);

		void line(int x1, int y1, int x2, int y2);
		void commit();

		range vrange() const;
		range hrange() const;

	private:
		const vector_rasterizer &operator =(const vector_rasterizer &);

		void hline(float tg, short ey, int x1, int x2);
		void jump(short x, short y);
		void jumpc(short x, short y);
		void extend_bounds(int x, int y);

	private:
		cells_container &_cells;
		int _min_x, _min_y, _max_x, _max_y;
		cell _current;
	};



	inline vector_rasterizer::range vector_rasterizer::vrange() const
	{	return std::make_pair(_min_y, _max_y);	}

	inline vector_rasterizer::range vector_rasterizer::hrange() const
	{	return std::make_pair(_min_x, _max_x);	}
}
