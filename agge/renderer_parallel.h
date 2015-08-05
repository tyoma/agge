#pragma once

#include "parallel.h"
#include "renderer.h"

namespace agge
{
	class renderer_parallel : noncopyable
	{
	public:
		explicit renderer_parallel(count_t parallelism);
		~renderer_parallel();

		template <typename BitmapT, typename MaskT, typename BlenderT, typename AlphaFn>
		void operator ()(BitmapT &bitmap, const rect_i *window, const MaskT &mask, const BlenderT &blender,
			const AlphaFn &alpha);

	private:
		raw_memory_object * const _scanline_caches; // Not an exception safe, but faster to place this one here...
		parallel _parallel;	// ... before this member, despite its constructor may throw.
		const count_t _parallelism;
	};



	inline renderer_parallel::renderer_parallel(count_t parallelism)
		: _scanline_caches(new raw_memory_object[parallelism]), _parallel(parallelism),
			_parallelism(parallelism)
	{	}

	inline renderer_parallel::~renderer_parallel()
	{	delete []_scanline_caches;	}

	template <typename BitmapT, typename MaskT, typename BlenderT, typename AlphaFn>
	void renderer_parallel::operator ()(BitmapT &bitmap, const rect_i *window, const MaskT &mask, const BlenderT &blender,
		const AlphaFn &alpha)
	{
		typedef renderer::adapter<BitmapT, BlenderT> rendition_adapter;

		const rendition_adapter ra(bitmap, window, blender);

		_parallel.call([&] (count_t i) {
			rendition_adapter ra_thread(ra);
			scanline_adapter<rendition_adapter> sl(ra_thread, _scanline_caches[i], mask.width());

			render(sl, mask, alpha, i, _parallelism);
		});
	}
}
