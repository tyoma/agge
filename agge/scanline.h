#pragma once

#include "config.h"
#include "types.h"

namespace agge
{
	class raw_memory_object
	{
	public:
		raw_memory_object();
		~raw_memory_object();

		template <typename T>
		T *get(count_t size);

	private:
		raw_memory_object(const raw_memory_object &other);
		const raw_memory_object &operator =(const raw_memory_object &rhs);

	private:
		uint8_t *_buffer;
		count_t _size;
	};


	template <typename RendererT>
	class scanline_adapter
	{
	public:
		typedef typename RendererT::cover_type cover_type;

	public:
		scanline_adapter(RendererT &renderer, raw_memory_object &covers_buffer, count_t max_length);

		bool begin(int y);
		void add_cell(int x, cover_type cover);
		void add_span(int x, unsigned int length, cover_type cover);
		void commit(int next_x = 0);

	private:
		scanline_adapter(const scanline_adapter &other);
		const scanline_adapter &operator =(const scanline_adapter &rhs);

	private:
		RendererT &_renderer;
		cover_type *_cover, *_start_cover;
		int _x, _start_x;
	};



	inline raw_memory_object::raw_memory_object()
		: _buffer(0), _size(0)
	{	}

	inline raw_memory_object::~raw_memory_object()
	{	delete []_buffer;	}

	template <typename T>
	inline T *raw_memory_object::get(count_t size)
	{
		size *= sizeof(T);
		size /= sizeof(uint8_t);
		if (size > _size)
		{
			uint8_t *buffer = new uint8_t[size];

			delete []_buffer;
			_buffer = buffer;
			_size = size;
			while (size--)
				*buffer++ = 0;
		}
		return reinterpret_cast<T *>(_buffer);
	}


	template <typename RendererT>
	inline scanline_adapter<RendererT>::scanline_adapter(RendererT &renderer, raw_memory_object &covers, count_t max_length)
		: _renderer(renderer), _x(0), _start_x(0)
	{
		_cover = _start_cover = covers.get<cover_type>(max_length + 16) + 4;
	}

	template <typename RendererT>
	inline bool scanline_adapter<RendererT>::begin(int y)
	{	return _renderer.set_y(y);	}

	template <typename RendererT>
	AGGE_INLINE void scanline_adapter<RendererT>::add_cell(int x, cover_type cover)
	{
		if (x != _x)
			commit(x);
		++_x;
		*_cover++ = cover;
	}

	template <typename RendererT>
	AGGE_INLINE void scanline_adapter<RendererT>::add_span(int x, unsigned int length, cover_type cover)
	{
		if (x != _x)
			commit(x);
		
		cover_type *p = _cover;
		
		_x += length;
		_cover += length;

		while (length--)
			*p++ = cover;
	}

	template <typename RendererT>
	AGGE_INLINE void scanline_adapter<RendererT>::commit(int next_x)
	{
		*reinterpret_cast<int *>(_cover) = 0;
		_renderer(_start_x, _x - _start_x, _start_cover);
		_start_x = _x = next_x;
		_cover = _start_cover;
	}
}
