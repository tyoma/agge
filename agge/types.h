#pragma once

namespace agge
{
	typedef float real_t;

	template <typename CoordT>
	struct point;

	template <typename CoordT>
	struct rect;

	typedef unsigned int count_t;
	typedef unsigned char uint8_t;
	typedef unsigned short uint16_t;
	typedef point<real_t> point_r;
	typedef rect<int> rect_i;

	enum bits_per_pixel { bpp32 = 32, bpp24 = 24, bpp16 = 16, bpp8 = 8 };

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

	class noncopyable
	{
	public:
		noncopyable() throw() { }

	private:
		noncopyable(const noncopyable &other);
		const noncopyable &operator =(const noncopyable &rhs);
	};
}
