#pragma once

#include "poly_buffer.h"

#include <atomic>

namespace agge
{
	template <typename BaseT>
	class queue
	{
	public:
      queue();

		template <typename T, typename PostProduceFn>
		void produce(T &value, const PostProduceFn &postproduce);

		template <typename ConsumerFn, typename PreConsumeFn>
		void consume(const ConsumerFn &consumer, const PreConsumeFn &preconsume);

   private:
      poly_buffer<BaseT> _buffer1, _buffer2;
      std::atomic<poly_buffer<BaseT> *> _write, _read;
      std::atomic<int> _count;
	};



   template <typename BaseT>
   inline queue<BaseT>::queue()
      : _buffer1(1000), _buffer2(1000), _write(&_buffer1), _read(&_buffer2)
   {  }

   template <typename BaseT>
   template <typename T, typename PostProduceFn>
   inline void queue<BaseT>::produce(T &value, const PostProduceFn &postproduce)
   {
      poly_buffer<BaseT> *write = _write.load();

      while (!_write.compare_exchange_strong(write, 0))
         write = _write.load();
      write->push_back(value);
      _write.store(write);
      postproduce(++_count);
   }

	template <typename BaseT>
	template <typename ConsumerFn, typename PreConsumeFn>
	inline void queue<BaseT>::consume(const ConsumerFn &consumer, const PreConsumeFn &preconsume)
	{
		preconsume(--_count);

      auto read = _read.load();

      if (read->empty())
      {
         const auto write = _read == &_buffer1 ? &_buffer2 : &_buffer1;

         for (auto w = write; !_write.compare_exchange_strong(w, read); )
         {  w = write;  }
         _read = read = write;
      }
      consumer(read->front());
      read->pop_front();
	}
}
