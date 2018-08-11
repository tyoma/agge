#include <agge.async/worker.h>

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	namespace
	{
		template <int N>
		class work : public worker< pair<int, double> >::work_in
		{
		public:
			work(double value)
				: _value(value)
			{	}

			virtual void run(worker< pair<int, double> >::out_queue_type &queue)
			{	queue.produce(make_pair(N, _value));	}

		private:
			double _value;
		};

		struct consumer
		{
			void operator ()(const pair<int, double> &value_) const
			{	value = value_;	}

			mutable pair<int, double> value;
		};
	}

	begin_test_suite( WorkerTests )
		test( WorkPutIsProcessedAndCanBeRead )
		{
			// INIT
			hybrid_event input_ready, output_ready;
			worker< pair<int, double> >::in_queue_type input(input_ready);
			worker< pair<int, double> >::out_queue_type output(output_ready);
			worker< pair<int, double> > w(input, output);
			consumer c;

			// ACT
			input.produce(work<12>(1.2345));

			// ACT / ASERT
			output.consume(c);

			// ASSERT
			assert_equal(make_pair(12, 1.2345), c.value);

			// ACT
			input.produce(work<13>(3.14956));

			// ACT / ASERT
			output.consume(c);

			// ASSERT
			assert_equal(make_pair(13, 3.14956), c.value);
		}
	end_test_suite
}
