#pragma once

namespace agge
{
	typedef float real;

	template <typename ClipperT, typename VectorRasterizerT>
	class rasterizer
	{
	public:
		typedef typename VectorRasterizerT::cell cell;

	public:
		void move_to(real x, real y);
		void line_to(real x, real y);
		void close_polygon();

		template <typename ScanlineAdapter, typename RendererT>
		void render(RendererT &renderer);
	};



	template <typename ClipperT, typename VectorRasterizerT>
	template <typename ScanlineAdapter, typename RendererT>
	inline void rasterizer<ClipperT, VectorRasterizerT>::render(RendererT &renderer)
	{
		unsigned char x = 254;
		renderer(1, 1, 1, &x);
	}
}
