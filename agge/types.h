#pragma once

namespace agge
{
	enum bits_per_pixel { bpp32 = 32, bpp24 = 24, bpp16 = 16, bpp8 = 8 };

	typedef unsigned int count_t;
	typedef unsigned char uint8_t;
	typedef unsigned short uint16_t;

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
	struct rect
	{
		CoordT x1, y1, x2, y2;
	};

	typedef rect<int> rect_i;


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
