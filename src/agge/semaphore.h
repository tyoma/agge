#pragma once

#include "handle.h"

#include <agge/types.h>

namespace agge
{
	class semaphore : noncopyable
	{
	public:
		semaphore();
		~semaphore();

		void signal();
		void wait();

	private:
		handle _handle;
	};
}
