#pragma once

#include <poly-queue/circular.h>

namespace agge
{
	template <typename T, typename EventT>
	class queue
	{
	public:
		queue(EventT &ready, int additional_limit = -1, EventT *additional_event = 0);

		template <typename FinalT>
		void produce(const FinalT &object);

		template <typename ConsumerT>
		bool consume(const ConsumerT &consumer);

		void stop();

	private:
		struct preconsumer
		{
		public:
			preconsumer(EventT &ready);

			bool operator ()(int n) const;
			void stop();

		private:
			EventT &_ready;
			bool _continue;
		};

		struct postproducer
		{
		public:
			postproducer(EventT &ready, int additional_limit, EventT *additional_event);

			void operator ()(int n) const;

		private:
			EventT & _ready, *_additional_event;
			int _additional_limit;
		};

	private:
		pq::circular_buffer< T, pq::poly_entry<T> > _inner;
		preconsumer _preconsumer;
		postproducer _postproducer;
	};



	template <typename T, typename EventT>
	inline queue<T, EventT>::preconsumer::preconsumer(EventT &ready)
		: _ready(ready), _continue(true)
	{	}

	template <typename T, typename EventT>
	inline bool queue<T, EventT>::preconsumer::operator ()(int n) const
	{
		if (!n)
			_ready.wait();
		return _continue;
	}

	template <typename T, typename EventT>
	inline void queue<T, EventT>::preconsumer::stop()
	{
		_continue = false;
		_ready.signal();
	}


	template <typename T, typename EventT>
	inline queue<T, EventT>::postproducer::postproducer(EventT &ready, int additional_limit, EventT *additional_event)
		: _ready(ready), _additional_event(additional_event), _additional_limit(!additional_event ? -1 : additional_limit)
	{	}

	template <typename T, typename EventT>
	inline void queue<T, EventT>::postproducer::operator ()(int n) const
	{
		if (!n)
			_ready.signal();
		if (n == _additional_limit)
			_additional_event->signal();
	}


	template <typename T, typename EventT>
	inline queue<T, EventT>::queue(EventT &ready, int additional_limit, EventT *additional_event)
		: _preconsumer(ready), _postproducer(ready, additional_limit, additional_event)
	{	}

	template <typename T, typename EventT>
	template <typename FinalT>
	inline void queue<T, EventT>::produce(const FinalT &object)
	{	_inner.produce(object, _postproducer);	}

	template <typename T, typename EventT>
	template <typename ConsumerT>
	inline bool queue<T, EventT>::consume(const ConsumerT &consumer)
	{	return _inner.consume(consumer, _preconsumer);	}

	template <typename T, typename EventT>
	inline void queue<T, EventT>::stop()
	{	_preconsumer.stop();	}
}
