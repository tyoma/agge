#pragma once

#include <agge/types.h>

#include <functional>
#include <memory>
#include <vector>

#if defined(_MSC_VER) && _MSC_VER==1500
	namespace std
	{
		using tr1::function;
		using tr1::shared_ptr;
	}
#endif

namespace agge
{
	class parallel
	{
	public:
		typedef std::function<void(count_t threadid)> kernel_function;

	public:
		explicit parallel(count_t parallelism);
		~parallel();

		void call(const kernel_function &kernel);

	private:
		struct thread;
		typedef std::shared_ptr<thread> thread_ptr;
		typedef std::vector<thread_ptr> threads;

	private:
		parallel(const parallel &other);
		const parallel &operator =(const parallel &rhs);

	private:
		threads _threads;
	};
}
