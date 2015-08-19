#include <agge/stroker.h>

#include "mocks.h"

#include <utee/ut/assert.h>
#include <utee/ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace
		{
			class passthrough_generator : public mocks::path
			{
			public:
				void add_vertex(real_t x, real_t y, int command)
				{
					point p = { x, y, command };
					points.push_back(p);
				}
			};
		}

		begin_test_suite( PathGeneratorAdapterTests )
			test( EmptySourcePathAddsNothingToGenerator )
			{
				// INIT
				mocks::path empty;
				passthrough_generator g;
				real_t x, y;

				// INIT / ACT
				path_generator_adapter<mocks::path, passthrough_generator> pg(empty, g);

				// ACT / ASSERT
				assert_equal(path_command_stop, pg.vertex(&x, &y));

				// ASSERT
				assert_is_empty(g.points);
			}

		end_test_suite
	}
}
