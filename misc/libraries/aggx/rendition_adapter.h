#pragma once

#include "basics.h"

namespace aggx
{
	template <class Bitmap, class Blender>
	class rendition_adapter
	{
	public:
		explicit rendition_adapter(Bitmap &target, const Blender &blender);

		void clear() const;
		void operator ()(int x, int y, int n, int8u cover) const;
		void operator ()(int x, int y, int n, const aggx::cover_type *covers) const;

	private:
		const Blender _blender;
		Bitmap &_target;
		int _width, _height;
	};



	template <class Bitmap, class Blender>
	inline rendition_adapter<Bitmap, Blender>::rendition_adapter(Bitmap &target, const Blender &blender)
		: _blender(blender), _target(target), _width(_target.width()), _height(_target.height())
	{	}

	template <class Bitmap, class Blender>
	inline void rendition_adapter<Bitmap, Blender>::clear() const
	{
		for (unsigned y = 0; y < _target.height(); ++y)
			_blender(_target.access(0, y), 0, y, _width);
	}

	template <class Bitmap, class Blender>
	void rendition_adapter<Bitmap, Blender>::operator ()(int x, int y, int n, int8u cover) const
	{
		if (y < 0 || _height <= y)
			return;
		if (x < 0)
		{
			n += x;
			x = 0;
		}
		if (x + n >= _width)
			n = _width() - x;
		if (n > 0)
			_blender(_target.access(x, y), x, y, n, cover);
	}

	template <class Bitmap, class Blender>
	inline void rendition_adapter<Bitmap, Blender>::operator ()(int x, int y, int n, const aggx::cover_type *covers) const
	{
		if (y < 0 || _height <= y)
			return;
		if (x < 0)
		{
			n += x;
			covers += -x;
			x = 0;
		}
		if (x + n >= _width)
			n = _width - x;
		if (n > 0)
			_blender(_target.access(x, y), x, y, n, covers);
	}
}
