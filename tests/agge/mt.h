#pragma once

#include <cstddef>

namespace agge
{
	namespace tests
	{
		typedef std::size_t thread_id;

		class mutex
		{
		public:
			mutex();
			~mutex();

			void lock();
			void unlock();

		private:
			char _buffer[100];
		};

		thread_id this_thread_id();
	}
}
