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
		void operator ()(BitmapT &bitmap_, const rect_i *window, const MaskT &mask, const BlenderT &blender,
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
	void renderer_parallel::operator ()(BitmapT &bitmap_, const rect_i *window, const MaskT &mask, const BlenderT &blender,
		const AlphaFn &alpha)
	{
		typedef renderer::adapter<BitmapT, BlenderT> rendition_adapter;

		struct kernel_function : parallel::kernel_function, noncopyable
		{
			kernel_function(const rendition_adapter &adapter_, const renderer_parallel &renderer_, const MaskT &mask_,
					const AlphaFn &alpha_)
				: adapter(adapter_), renderer(renderer_), mask(mask_), alpha(alpha_)
			{	}

			virtual void operator ()(count_t threadid)
			{
				rendition_adapter ra_thread(adapter);
				scanline_adapter<rendition_adapter> sl(ra_thread, renderer._scanline_caches[threadid], mask.width());

				render(sl, mask, alpha, threadid, renderer._parallelism);
			}

			const rendition_adapter &adapter;
			const renderer_parallel &renderer;
			const MaskT &mask;
			const AlphaFn &alpha;
		};

		const rendition_adapter adapter(bitmap_, window, blender);
		kernel_function kernel(adapter, *this, mask, alpha);

		_parallel.call(kernel);
	}
}
