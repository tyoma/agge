#include <agge/parallel.h>

#include "mt.h"

#include <utee/ut/assert.h>
#include <utee/ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		begin_test_suite( ParallelExecutionTests )
			
			class thread_capture : public parallel::kernel_function
			{
			public:
				typedef vector< pair<count_t /*logical_id*/, thread_id /*physical_id*/> > log_container;

			public:
				thread_capture(log_container &log_, mutex &mtx)
					: log(log_), _mutex(mtx)
				{	}

				virtual void operator ()(count_t logical_id)
				{
					_mutex.lock();
					log.push_back(make_pair(logical_id, this_thread_id()));
					_mutex.unlock();
				}

			public:
				log_container &log;

			private:
				const thread_capture &operator =(const thread_capture &rhs);

			private:
				mutex &_mutex;
			};

			struct second_equal
			{
				second_equal(thread_id id_)
					: id(id_)
				{	}

				bool operator ()(const pair<count_t, thread_id> &v) const
				{	return v.second == id;	}

				thread_id id;
			};


			test( CreateDestroyParallelObjectsOfDifferentParallelism )
			{
				// INIT / ACT / ASSERT
				parallel p1(1);
				parallel p2(2);
				parallel p3(3);
				parallel p10(10);
			}

			
			test( CallsInSingleThreadedObjectAreMadeInTheCurrentThread )
			{
				// INIT
				parallel p(1);
				thread_capture::log_container log;
				mutex mtx;
				thread_capture tc(log, mtx);

				// ACT
				p.call(tc);

				// ASSERT
				assert_equal(1u, log.size());
				assert_equal(0u, log[0].first);
				assert_equal(this_thread_id(), log[0].second);

				// ACT
				p.call(tc);

				// ASSERT
				assert_equal(2u, log.size());
				assert_equal(0u, log[1].first);
				assert_equal(this_thread_id(), log[1].second);
			}

			
			test( MultithreadedParallelObjectMakesCallInDifferentThreads )
			{
				// INIT
				parallel p3(3), p7(7);
				thread_capture::log_container log;
				mutex mtx;
				thread_capture tc(log, mtx);

				// ACT
				p3.call(tc);

				// ASSERT
				sort(log.begin(), log.end());

				assert_equal(3u, log.size());
				assert_equal(this_thread_id(), log[0].second);
				for (count_t i = 0; i < log.size(); ++i)
				{
					assert_equal(i, log[i].first);
					assert_equal(1, count_if(log.begin(), log.end(), second_equal(log[i].second)));
				}

				// INIT
				log.clear();

				// ACT
				p7.call(tc);

				// ASSERT
				sort(log.begin(), log.end());

				assert_equal(7u, log.size());
				assert_equal(this_thread_id(), log[0].second);
				for (count_t i = 0; i < log.size(); ++i)
				{
					assert_equal(i, log[i].first);
					assert_equal(1, count_if(log.begin(), log.end(), second_equal(log[i].second)));
				}
			}

			
			test( ThreadsArePersistentInParallelObject )
			{
				// INIT
				parallel p1(4), p2(4);
				thread_capture::log_container log1, log2, log3, log4;
				mutex mtx;
				thread_capture tc1(log1, mtx);
				thread_capture tc2(log2, mtx);
				thread_capture tc3(log3, mtx);
				thread_capture tc4(log4, mtx);
				
				// ACT
				p1.call(tc1);
				p1.call(tc2);
				p2.call(tc3);
				p1.call(tc4);

				// ASSERT
				sort(log1.begin(), log1.end());
				sort(log2.begin(), log2.end());
				sort(log3.begin(), log3.end());
				sort(log4.begin(), log4.end());

				assert_equal(log1, log2);
				assert_equal(log1, log4);
				assert_not_equal(log1, log3);
			}

		end_test_suite
	}
}
