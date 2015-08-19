#pragma once

#include "types.h"

namespace agge
{
	template <typename SourceT, typename GeneratorT>
	class path_generator_adapter
	{
	public:
		path_generator_adapter(SourceT &source, GeneratorT &generator);

		void rewind(int path_id);
		int vertex(real_t *x, real_t *y);
	};



	template <typename SourceT, typename GeneratorT>
	inline path_generator_adapter<SourceT, GeneratorT>::path_generator_adapter(SourceT &/*source*/, GeneratorT &/*generator*/)
	{	}

	template <typename SourceT, typename GeneratorT>
	inline int path_generator_adapter<SourceT, GeneratorT>::vertex(real_t * /*x*/, real_t * /*y*/)
	{
		return 0;
	}
}
