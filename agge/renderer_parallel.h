#pragma once

#include "parallel.h"
#include "renderer.h"

namespace agge
{
	class renderer_parallel
	{
	public:
		renderer_parallel(count_t parallelism);

		template <typename BitmapT, typename MaskT, typename BlenderT, typename AlphaFn>
		void operator ()(BitmapT &bitmap, const rect_i *window, const MaskT &mask, const BlenderT &blender, const AlphaFn &alpha);
	};
}
