#pragma once

#include "poly_buffer.h"

namespace agge
{
	template <typename BaseT>
	class queue
	{
	public:
		template <typename T, typename PostProduceFn>
		void produce(const T &value, const PostProduceFn &postproduce);

		template <typename ConsumerFn, typename PreConsumeFn>
		void consume(const ConsumerFn &consumer, const PreConsumeFn &preconsume);
	};



	template <typename BaseT>
	template <typename ConsumerFn, typename PreConsumeFn>
	inline void queue<BaseT>::consume(const ConsumerFn &/*consumer*/, const PreConsumeFn &preconsume)
	{
		preconsume(-1);
	}
}
