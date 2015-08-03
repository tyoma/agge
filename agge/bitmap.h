#pragma once

#include "types.h"

namespace agge
{
	template <typename PixelT>
	struct pixel_bpp
	{	};


	template <typename PixelT, typename RawBitmapT>
	class bitmap : public RawBitmapT
	{
	public:
		typedef PixelT pixel;

	public:
		bitmap(count_t width, count_t height);

		pixel *row_ptr(count_t y);
		const pixel *row_ptr(count_t y) const;
	};



	template <>
	struct pixel_bpp<pixel32>
	{	static const bits_per_pixel bpp = bpp32;	};

	template <>
	struct pixel_bpp<pixel24>
	{	static const bits_per_pixel bpp = bpp24;	};

	template <>
	struct pixel_bpp<pixel16>
	{	static const bits_per_pixel bpp = bpp16;	};

	template <>
	struct pixel_bpp<uint8_t>
	{	static const bits_per_pixel bpp = bpp8;	};


	template <typename PixelT, typename RawBitmapT>
	inline bitmap<PixelT, RawBitmapT>::bitmap(count_t width, count_t height)
		: RawBitmapT(width, height, pixel_bpp<PixelT>::bpp)
	{	}

	template <typename PixelT, typename RawBitmapT>
	inline typename bitmap<PixelT, RawBitmapT>::pixel *bitmap<PixelT, RawBitmapT>::row_ptr(count_t y)
	{	return static_cast<pixel *>(RawBitmapT::row_ptr(y));	}

	template <typename PixelT, typename RawBitmapT>
	inline const typename bitmap<PixelT, RawBitmapT>::pixel *bitmap<PixelT, RawBitmapT>::row_ptr(count_t y) const
	{	return static_cast<const pixel *>(RawBitmapT::row_ptr(y));	}
}
