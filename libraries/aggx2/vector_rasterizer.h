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
			_1_shift = 8,
			_ep_shift = _1_shift + 4,

			_1 = 1 << _1_shift,
			_1_mask = _1 - 1,
			_ep = 1 << _ep_shift,
		};

#pragma pack(push, 1)
		struct cell
		{
			short x, y;
			int area;
			short cover;

			static const cell empty;
		};
#pragma pack(pop)

		typedef std::vector<cell> cells_container;
		typedef std::pair<int, int> range;

	public:
		explicit vector_rasterizer(cells_container &cells);
		
		void reset();

		void line(int x1, int y1, int x2, int y2);
		void commit();

		range vrange() const;
		range hrange() const;

	private:
		const vector_rasterizer &operator =(const vector_rasterizer &);

		void hline(int tg, short ey, int x1, int x2, int dy);
		void jump_xy(short x, short y);
		void jump_x(short x);
		void jumpc(short x, short y);
		void extend_bounds(short x, short y);

	private:
		cells_container &_cells;
		short _min_x, _min_y, _max_x, _max_y;
		cell _current;
	};



	inline vector_rasterizer::range vector_rasterizer::vrange() const
	{	return std::make_pair(_min_y, _max_y);	}

	inline vector_rasterizer::range vector_rasterizer::hrange() const
	{	return std::make_pair(_min_x, _max_x);	}
}
