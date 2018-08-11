#include <agge.async/queue.h>

#include <ut/assert.h>
#include <ut/test.h>

namespace agge
{
	namespace tests
	{
		namespace mocks
		{
			struct function0
			{
				virtual void operator ()()
				{	}
			} c_none;

			class event
			{
			public:
				event(function0 &onsignal = c_none, function0 &onwait = c_none)
					: waited(0), signalled(0), _onsignal(onsignal), _onwait(onwait), _signalled(false)
				{	}

				void wait()
				{
					const bool require_signalling = !_signalled;

					_onwait();
					assert_is_true(!require_signalling || _signalled);
					++waited;
					_signalled = false;
				}

				void signal()
				{
					_signalled = true;
					_onsignal();
					++signalled;
				}

			public:
				unsigned waited, signalled;

			private:
				function0 &_onsignal, &_onwait;
				bool _signalled;
			};
		}

		namespace
		{
			class work
			{
			public:
				virtual ~work() {	}
				virtual void operator ()(int &result) = 0;
			};

			template <int N>
			class real_work : public work
			{
			public:
				real_work(int multiplier = 0)
					: _multiplier(multiplier)
				{	}

				virtual void operator ()(int &result)
				{	result = N * _multiplier;	}

			private:
				int _multiplier;
			};

			template <int N>
			class real_work_nc : public work
			{
			public:
				real_work_nc(int multiplier = 0)
					: _multiplier(multiplier)
				{	}

				real_work_nc(real_work_nc &other)
					: _multiplier(other._multiplier)
				{	}

				virtual void operator ()(int &result)
				{	result = N * _multiplier;	}

			private:
				int _multiplier;
			};

			class consumer
			{
			public:
				consumer()
					: result(-1)
				{	}

				void operator ()(work &object) const
				{	object(result);	}

			public:
				mutable int result;
			};
		}

		begin_test_suite( QueueTests )
			struct throwing_function : mocks::function0
			{
				virtual void operator ()()
				{	throw 0;	}
			};

			test( ConsumeStartsWaitingWhenQueueIsEmpty )
			{
				// INIT
				throwing_function throwing;
				mocks::event e(mocks::c_none, throwing);
				queue<work, mocks::event> q(e);
				consumer c;

				// ACT / ASSERT
				assert_throws(q.consume(c), int);
			}


			template <typename QueueT, typename ObjectT>
			class producing_function : public mocks::function0
			{
			public:
				producing_function(const ObjectT &object)
					: queue(0), event_to_check(0), _object(object)
				{	}

				virtual void operator ()()
				{
					queue->produce(_object);
					assert_equal(1u, event_to_check->signalled);
				}

			public:
				QueueT *queue;
				mocks::event *event_to_check;

			private:
				ObjectT _object;
			};

			test( FirstProduceSignalsTransitionToNonEmptyIfWaited )
			{
				// INIT
				throwing_function throwing;
				producing_function< queue<work, mocks::event>, real_work<13> > producing(real_work<13>(3));
				mocks::event e(mocks::c_none, producing);
				queue<work, mocks::event> q(e);
				consumer c;

				producing.queue = &q;
				producing.event_to_check = &e;

				// ACT
				q.consume(c);

				// ASSERT
				assert_equal(39, c.result);
				assert_equal(1u, e.signalled);
			}


			test( ProduceDoesNotSignalTransitionToNonEmptyIfNotWaited )
			{
				// INIT
				mocks::event e;
				queue<work, mocks::event> q(e);

				// ACT
				q.produce(real_work<13>());
				q.produce(real_work<17>());
				q.produce(real_work<19>());

				// ASSERT
				assert_equal(0u, e.signalled);
			}


			test( ProducedObjectsAreConsumedWithoutWaiting )
			{
				// INIT
				mocks::event e;
				queue<work, mocks::event> q(e);
				consumer c;

				q.produce(real_work<13>(2));
				q.produce(real_work<17>(3));
				q.produce(real_work<19>(1));

				// ACT / ASSERT
				assert_is_true(q.consume(c));

				// ASSERT
				assert_equal(26, c.result);

				// ACT / ASSERT
				assert_is_true(q.consume(c));

				// ASSERT
				assert_equal(51, c.result);

				// ACT / ASSERT
				assert_is_true(q.consume(c));

				// ASSERT
				assert_equal(19, c.result);
				assert_equal(0u, e.signalled);
				assert_equal(0u, e.waited);
			}


			test( StoppingQueueSignalsReadyEvent )
			{
				// INIT
				mocks::event e;
				queue<work, mocks::event> q(e);

				// ACT
				q.stop();

				// ASSERT
				assert_equal(1u, e.signalled);

				// ACT
				q.stop();

				// ASSERT
				assert_equal(2u, e.signalled);
			}


			test( ConsumeReturnsFalseOnStoppedQueue )
			{
				// INIT
				mocks::event e;
				queue<work, mocks::event> q(e);
				consumer c;

				// ACT
				q.stop();

				// ACT / ASSERT
				assert_is_false(q.consume(c));

				// ASSERT
				assert_equal(1u, e.waited);
			}


			test( ConsumeReturnsFalseAndReadsNothingOnStoppedQueue )
			{
				// INIT
				mocks::event e;
				queue<work, mocks::event> q(e);
				consumer c;

				q.produce(real_work<7>(1));
				q.stop();

				// ACT / ASSERT
				assert_is_false(q.consume(c));

				// ASSERT
				assert_equal(-1, c.result);
			}


			template <typename QueueT>
			struct stopping_function : mocks::function0
			{
				virtual void operator ()()
				{	queue->stop();	}

				QueueT *queue;
			};

			test( ConsumeReturnsFalseWhenStoppingQueueOnWait )
			{
				// INIT
				stopping_function< queue<work, mocks::event> > stopping;
				mocks::event e(mocks::c_none, stopping);
				queue<work, mocks::event> q(e);
				consumer c;

				stopping.queue = &q;

				// ACT / ASSERT
				assert_is_false(q.consume(c));
			}


			template <typename QueueT>
			struct is_stopped_function : mocks::function0
			{
				virtual void operator ()()
				{
					consumer c;

					assert_is_false(queue->consume(c));
				}

				QueueT *queue;
			};

			test( QueueCanStopWhenReadyIsSignalledForStopping )
			{
				// This test basically verifies that _continue flag is reset before the 'ready' event gets signalled.
				// INIT
				is_stopped_function< queue<work, mocks::event> > is_stopped;
				mocks::event e(is_stopped);
				queue<work, mocks::event> q(e);

				is_stopped.queue = &q;

				// ACT / ASSERT
				q.stop();

				// ASSERT
				assert_equal(1u, e.signalled);
				assert_equal(1u, e.waited);
			}


			test( AdditionalEventIsRaisedWhenLimitIsReached )
			{
				// INIT
				mocks::event e, additional1, additional2;
				queue<work, mocks::event> q1(e, 3, &additional1), q2(e, 2, &additional2);

				// ACT
				q1.produce(real_work<1>());
				q1.produce(real_work<2>());
				q2.produce(real_work<2>());

				// ASSERT
				assert_equal(0u, additional1.signalled);
				assert_equal(0u, additional1.signalled);

				// ACT
				q1.produce(real_work<1>());
				q2.produce(real_work<2>());

				// ASSERT
				assert_equal(0u, additional1.waited);
				assert_equal(1u, additional1.signalled);
				assert_equal(0u, additional2.waited);
				assert_equal(1u, additional2.signalled);
			}


			test( AdditionalEventIsWaitedWhenDroppingFromTheLimit )
			{
				// INIT
				mocks::event e, additional1, additional2;
				queue<work, mocks::event> q1(e, 3, &additional1), q2(e, 2, &additional2);
				consumer c;

				q1.produce(real_work<1>());
				q1.produce(real_work<2>());
				q1.produce(real_work<2>());
				q2.produce(real_work<2>());
				q2.produce(real_work<2>());

				// ACT
				q1.consume(c);

				// ASSERT
				assert_equal(1u, additional1.waited);

				// ACT
				q1.consume(c);

				// ASSERT
				assert_equal(1u, additional1.waited);

				// ACT
				q2.consume(c);

				// ASSERT
				assert_equal(1u, additional2.waited);

				// ACT
				q2.consume(c);

				// ASSERT
				assert_equal(1u, additional2.waited);
			}


			test( ObjectConstructedFromNonConstRefCanBeProduced )
			{
				// INIT
				mocks::event e;
				queue<work, mocks::event> q(e);
				consumer c;
				real_work_nc<13> v1(2);
				real_work_nc<17> v2(3);
				real_work_nc<19> v3(1);

				// ACT
				q.produce(v1);
				q.produce(v2);
				q.produce(v3);

				// ACT / ASSERT
				assert_is_true(q.consume(c));

				// ASSERT
				assert_equal(26, c.result);

				// ACT / ASSERT
				assert_is_true(q.consume(c));

				// ASSERT
				assert_equal(51, c.result);

				// ACT / ASSERT
				assert_is_true(q.consume(c));

				// ASSERT
				assert_equal(19, c.result);
			}
		end_test_suite
	}
}
