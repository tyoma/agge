#pragma once

#include <agge/hybrid_event.h>
#include <agge/thread.h>
#include <poly-queue/circular.h>

namespace agge
{
	template <typename WorkT>
	class processor : noncopyable
	{
	public:
		typedef pq::circular_buffer< WorkT, pq::poly_entry<WorkT> > input_queue_t;
		typedef pq::circular_buffer< WorkT, pq::poly_entry<WorkT> > output_queue_t;

	public:
		processor(input_queue_t &input, hybrid_event &has_input, output_queue_t &output, hybrid_event &output_ready);
		~processor();

	private:
		class preconsumer;
		class consumer;
		class postproducer;

	private:
		static void worker(void *self);

	private:
		input_queue_t &_input;
		hybrid_event &_has_input;
		output_queue_t &_output;
		hybrid_event &_output_ready;
		thread _thread;
		bool _continue;
	};

	template <typename WorkT>
	class processor<WorkT>::preconsumer
	{
	public:
		preconsumer(hybrid_event &e, bool &continue_)
			: _e(e), _continue(continue_)
		{	}

		bool operator ()(int n) const
		{
			if (!n)
				_e.wait();
			return _continue;
		}

	private:
		hybrid_event &_e;
		bool &_continue;
	};

	template <typename WorkT>
	class processor<WorkT>::consumer
	{
	public:
		consumer(typename processor<WorkT>::output_queue_t &queue, typename processor<WorkT>::postproducer &pp)
			: _queue(queue), _pp(pp)
		{	}

		void operator ()(WorkT &work) const
		{
			work();
			_queue.produce(work, _pp);
		}

	private:
		typename processor<WorkT>::output_queue_t &_queue;
		typename processor<WorkT>::postproducer &_pp;
	};

	template <typename WorkT>
	class processor<WorkT>::postproducer
	{
	public:
		postproducer(hybrid_event &e)
			: _e(e)
		{	}

		void operator ()(int n) const
		{
			if (!n)
				_e.signal();
		}

	private:
		hybrid_event &_e;
	};


	template <typename WorkT>
	inline processor<WorkT>::processor(input_queue_t &input, hybrid_event &has_input,
			output_queue_t &output, hybrid_event &output_ready)
		: _input(input), _has_input(has_input), _output(output), _output_ready(output_ready), _thread(&worker, this),
			_continue(true)
	{	}

	template <typename WorkT>
	inline processor<WorkT>::~processor()
	{
		_continue = false;
		_has_input.signal();
	}

	template <typename WorkT>
	inline void processor<WorkT>::worker(void * self_)
	{
		processor &self = *static_cast<processor *>(self_);
		preconsumer pc(self._has_input, self._continue);
		postproducer pp(self._output_ready);
		consumer c(self._output, pp);

		while (self._input.consume(c, pc))
		{	}
	}
}
