#pragma once

namespace agge
{
	typedef float real;

	template <typename ClipperT>
	class rasterizer
	{
	public:
		void move_to(real x, real y);
		void line_to(real x, real y);
		void close_polygon();

		template <typename BitmapT, typename BlenderT, typename AlphaFn>
		void render(BitmapT &bitmap, const BlenderT &blender, const AlphaFn &alpha);
	};
}
