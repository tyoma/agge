#pragma once

#include "config.h"

#include <algorithm>
#include <vector>

namespace agge
{
	template <typename RendererT>
	class scanline_adapter
	{
	public:
		typedef typename RendererT::cover_type cover_type;
		typedef std::vector<cover_type> covers_buffer_type;

	public:
		scanline_adapter(RendererT &renderer, covers_buffer_type &covers, size_t max_length);

		void begin(unsigned int y);
		void add_cell(unsigned int x, cover_type cover);
		void add_span(unsigned int x, unsigned int length, cover_type cover);
		void commit(unsigned int next_x = 0);

	private:
		scanline_adapter(const scanline_adapter &other);
		const scanline_adapter &operator =(const scanline_adapter &rhs);

	private:
		RendererT &_renderer;
		covers_buffer_type &_covers;
		cover_type *_cover, *_start_cover;
		unsigned int _y, _x, _start_x;
	};



	template <typename RendererT>
	inline scanline_adapter<RendererT>::scanline_adapter(RendererT &renderer, covers_buffer_type &covers, size_t max_length)
		: _renderer(renderer), _covers(covers)
	{
		max_length += 16;
		if (max_length > _covers.size())
			_covers.resize(max_length);
		_start_cover = &_covers[3];
	}

	template <typename RendererT>
	inline void scanline_adapter<RendererT>::begin(unsigned int y)
	{
		_y = y;
		_start_x = _x = 0;
		_cover = _start_cover;
	}

	template <typename RendererT>
	AGGE_INLINE void scanline_adapter<RendererT>::add_cell(unsigned int x, cover_type cover)
	{
		if (x != _x)
			commit(x);
		++_x;
		*_cover++ = cover;
	}

	template <typename RendererT>
	AGGE_INLINE void scanline_adapter<RendererT>::add_span(unsigned int x, unsigned int length, cover_type cover)
	{
		if (x != _x)
			commit(x);
		
		cover_type *p = _cover;
		
		_x += length;
		_cover += length;

		std::fill_n(p, length, cover);
	}

	template <typename RendererT>
	AGGE_INLINE void scanline_adapter<RendererT>::commit(unsigned int next_x)
	{
		*reinterpret_cast<int *>(_cover) = 0;
		_renderer(_start_x, _y, _x - _start_x, _start_cover);
		_start_x = _x = next_x;
		_cover = _start_cover;
	}
}
