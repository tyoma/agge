#pragma once

#include <polyq/queue.h>

#include <agge/hybrid_event.h>
#include <agge/thread.h>

namespace agge
{
	template <typename WorkOutT>
	class worker : noncopyable
	{
	public:
		struct work_in;
		typedef WorkOutT work_out;
		typedef polyq::queue<work_in, hybrid_event> in_queue_type;
		typedef polyq::queue<work_out, hybrid_event> out_queue_type;

	public:
		worker(in_queue_type &input, out_queue_type &output);
		~worker();

	private:
		class consumer;

	private:
		static void worker_proc(void *self);

	private:
		in_queue_type &_input;
		out_queue_type &_output;
		thread _worker_thread;
	};

	template <typename WorkOutT>
	struct worker<WorkOutT>::work_in
	{
		virtual ~work_in() {	}
		virtual void run(typename worker<WorkOutT>::out_queue_type &queue) = 0;
	};

	template <typename WorkOutT>
	class worker<WorkOutT>::consumer
	{
	public:
		consumer(typename worker<WorkOutT>::out_queue_type &queue)
			: _queue(queue)
		{	}

		void operator ()(work_in &w) const
		{	w.run(_queue);	}

	private:
		const consumer &operator =(const consumer &rhs);

	private:
		typename worker<WorkOutT>::out_queue_type &_queue;
	};


#pragma warning(disable: 4355)
	template <typename WorkOutT>
	inline worker<WorkOutT>::worker(in_queue_type &input, out_queue_type &output)
		: _input(input), _output(output), _worker_thread(&worker::worker_proc, this)
	{	}
#pragma warning(default: 4355)

	template <typename WorkOutT>
	inline worker<WorkOutT>::~worker()
	{	_input.stop();	}

	template <typename WorkOutT>
	inline void worker<WorkOutT>::worker_proc(void *self_)
	{
		worker *self = static_cast<worker *>(self_);
		consumer c(self->_output);

		while (self->_input.consume(c))
		{	}
	}
}
