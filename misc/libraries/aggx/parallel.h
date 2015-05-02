#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace aggx
{
	class parallel
	{
	public:
		class event;
		struct worker_data;
		typedef std::function<void(unsigned int threadid)> kernel_function;

	public:
		parallel(unsigned int threads);
		~parallel();

		void call(const kernel_function &kernel);

	private:
		typedef std::shared_ptr<void> handle;
		typedef std::pair<handle, worker_data> thread_data;
		typedef std::vector<thread_data> threads;

	private:
		threads _threads;
	};
}
