#pragma once

namespace aggx
{
	typedef float real;

	typedef signed char int8;
	typedef unsigned char int8u;
	typedef signed short int int16;
	typedef unsigned short int int16u;
	typedef signed int int32;
	typedef unsigned int int32u;
	typedef unsigned char cover_type;

	template <class T>
	struct point_base;
	template <class T>
	struct rect_base;

	typedef point_base<real> point_r;
	typedef rect_base<int> rect_i;
	typedef rect_base<real> rect_r;

	template <class T>
	struct point_base
	{
		typedef T value_type;

		T x,y;

		point_base()
		{	}

		point_base(T x_, T y_)
			: x(x_), y(y_)
		{	}
	};

	template <class T>
	struct rect_base
	{
		typedef T value_type;
		typedef rect_base<T> self_type;

		T x1, y1, x2, y2;

		rect_base()
		{	}

		rect_base(T x1_, T y1_, T x2_, T y2_)
			: x1(x1_), y1(y1_), x2(x2_), y2(y2_)
		{	}

		void init(T x1_, T y1_, T x2_, T y2_) 
		{
			x1 = x1_; y1 = y1_; x2 = x2_; y2 = y2_; 
		}

		const self_type& normalize()
		{
			T t;
			if(x1 > x2) { t = x1; x1 = x2; x2 = t; }
			if(y1 > y2) { t = y1; y1 = y2; y2 = t; }
			return *this;
		}

		bool clip(const self_type& r)
		{
			if(x2 > r.x2) x2 = r.x2;
			if(y2 > r.y2) y2 = r.y2;
			if(x1 < r.x1) x1 = r.x1;
			if(y1 < r.y1) y1 = r.y1;
			return x1 <= x2 && y1 <= y2;
		}

		bool is_valid() const
		{
			return x1 <= x2 && y1 <= y2;
		}

		bool hit_test(T x, T y) const
		{
			return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
		}
	};

	//----------------------------------------------------poly_subpixel_scale_e
	// These constants determine the subpixel accuracy, to be more precise, 
	// the number of bits of the fractional part of the coordinates. 
	// The possible coordinate capacity in bits can be calculated by formula:
	// sizeof(int) * 8 - poly_subpixel_shift, i.e, for 32-bit integers and
	// 8-bits fractional part the capacity is 24 bits.
	enum poly_subpixel_scale_e
	{
		poly_subpixel_shift = 8,                      //----poly_subpixel_shift
		poly_subpixel_scale = 1 << poly_subpixel_shift, //----poly_subpixel_scale 
		poly_subpixel_mask  = poly_subpixel_scale - 1,  //----poly_subpixel_mask 
	};

	//----------------------------------------------------------filling_rule_e
	enum filling_rule_e
	{
		fill_non_zero,
		fill_even_odd
	};

	enum path_commands_e
	{
		path_cmd_stop     = 0,        //----path_cmd_stop    
		path_cmd_move_to  = 1,        //----path_cmd_move_to 
		path_cmd_line_to  = 2,        //----path_cmd_line_to 
		path_cmd_curve3   = 3,        //----path_cmd_curve3  
		path_cmd_curve4   = 4,        //----path_cmd_curve4  
		path_cmd_curveN   = 5,        //----path_cmd_curveN
		path_cmd_catrom   = 6,        //----path_cmd_catrom
		path_cmd_ubspline = 7,        //----path_cmd_ubspline
		path_cmd_end_poly = 0x0F,     //----path_cmd_end_poly
		path_cmd_mask     = 0x0F      //----path_cmd_mask    
	};

	enum path_flags_e
	{
		path_flags_none  = 0,         //----path_flags_none 
		path_flags_ccw   = 0x10,      //----path_flags_ccw  
		path_flags_cw    = 0x20,      //----path_flags_cw   
		path_flags_close = 0x40,      //----path_flags_close
		path_flags_mask  = 0xF0       //----path_flags_mask 
	};

	const real pi = 3.14159265358979323846f;

	struct rgba8
	{
		typedef int8u value_type;
		typedef int32u calc_type;
		typedef int32 long_type;

		enum base_scale_e
		{
			base_shift = 8,
			base_scale = 1 << base_shift,
			base_mask  = base_scale - 1
		};
		typedef rgba8 self_type;

		value_type r;
		value_type g;
		value_type b;
		value_type a;

		rgba8(unsigned r_, unsigned g_, unsigned b_, unsigned a_ = base_mask)
			: r(value_type(r_)),  g(value_type(g_)),  b(value_type(b_)),  a(value_type(a_))
		{	}
	};

	struct order_bgra { enum bgra_e { B=0, G=1, R=2, A=3, rgba_tag }; }; //----order_bgra

	inline int uround(real v)
	{	return unsigned(v + 0.5f);	}

	inline int iround(real v)
	{	return int(v < 0.0 ? v - 0.5f : v + 0.5f);	}

	inline real deg2rad(real deg)
	{	return deg * pi / 180.0f;	}


	inline bool is_stop(unsigned c)
	{	return c == path_cmd_stop;	}

	inline bool is_move_to(unsigned c)
	{	return c == path_cmd_move_to;	}

	inline bool is_vertex(unsigned c)
	{	return c >= path_cmd_move_to && c < path_cmd_end_poly;	}

	inline bool is_close(unsigned c)
	{	return (c & ~(path_flags_cw | path_flags_ccw)) == (path_cmd_end_poly | path_flags_close);	}

	inline bool is_end_poly(unsigned c)
	{	return (c & path_cmd_mask) == path_cmd_end_poly;	}

	inline unsigned get_close_flag(unsigned c)
	{	return c & path_flags_close;	}
}
