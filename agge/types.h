#pragma once

namespace agge
{
	enum bits_per_pixel {
		bpp32 = 32,
		bpp24 = 24,
		bpp16 = 16,
		bpp8 = 8
	};

	enum path_commands {
		path_command_stop = 0x00,
		path_command_move_to = 0x01,
		path_command_line_to = 0x02,
		path_command_end_poly = 0x0F,
		path_commands_mask = 0x0F,

		path_flag_close = 0x10,
		path_flags_mask = 0xF0
	};


	typedef unsigned int count_t;
	typedef unsigned char uint8_t;
	typedef unsigned short uint16_t;
	typedef float real_t;

#pragma pack(push, 1)
	struct pixel32
	{
		uint8_t c0, c1, c2, c3;
	};

	struct pixel24
	{
		uint8_t c0, c1, c2;
	};

	struct pixel16
	{
		unsigned int c0 : 5;
		unsigned int c1 : 5;
		unsigned int c2 : 5;
	};
#pragma pack(pop)

	template <typename CoordT>
	struct point
	{
		CoordT x, y;
	};

	template <typename CoordT>
	struct rect
	{
		CoordT x1, y1, x2, y2;
	};


	typedef point<real_t> point_r;
	typedef rect<int> rect_i;


	template <typename CoordT>
	point<CoordT> create_point(CoordT x, CoordT y)
	{
		point<CoordT> p = { x, y };
		return p;
	}

	template <typename CoordT>
	inline CoordT width(const rect<CoordT> &rc)
	{	return rc.x2 - rc.x1;	}

	template <typename CoordT>
	inline CoordT height(const rect<CoordT> &rc)
	{	return rc.y2 - rc.y1;	}

	class noncopyable
	{
	public:
		noncopyable() throw() { }

	private:
		noncopyable(const noncopyable &other);
		const noncopyable &operator =(const noncopyable &rhs);
	};
}
