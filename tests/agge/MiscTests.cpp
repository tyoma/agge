#include <agge/math.h>
#include <agge/memory.h>
#include <agge/path.h>
#include <agge/tools.h>

#include "helpers.h"

#include <algorithm>
#include <tests/common/helpers.h>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		begin_test_suite( MiscTests )

			test( RawMemoryObjectReturnsNonNullAddress )
			{
				// INIT / ACT
				raw_memory_object rmo;

				// ACT / ASSERT
				assert_not_null(rmo.get<int>(1));
				assert_not_null(rmo.get<double>(5));
			}


			test( RawMemoryObjectReturnsTheSameBlockForTheSameSize )
			{
				// INIT / ACT
				raw_memory_object rmo1, rmo2;

				// ACT
				assert_equal(rmo1.get<int>(103), rmo1.get<int>(103));
				assert_equal(rmo2.get<uint8_t>(721), rmo2.get<uint8_t>(721));
			}


			test( RawMemoryObjectAllocatesNewChunkForALargerSize )
			{
				// INIT
				raw_memory_object rmo;

				// ACT
				char *bufferChars = rmo.get<char>(100);
				double *bufferDoubles1 = rmo.get<double>(100);

				// ASSERT
				assert_not_equal(reinterpret_cast<void *>(bufferChars), reinterpret_cast<void *>(bufferDoubles1));

				// ACT
				double *bufferDoubles2 = rmo.get<double>(150);

				// ASSERT
				assert_not_equal(bufferDoubles1, bufferDoubles2);

				// ACT
				double *bufferDoubles3 = rmo.get<double>(151);

				// ASSERT
				assert_not_equal(bufferDoubles2, bufferDoubles3);
			}


			test( MemoryIsZeroedAfterReallocation )
			{
				// INIT
				raw_memory_object rmo;

				*(rmo.get<uint8_t>(100) + 5) = 33;

				// ACT
				uint8_t *p = rmo.get<uint8_t>(101);

				// ASSERT
				assert_equal(0, *p);
				assert_equal(p, max_element(p, p + 101));
			}


			test( RawMemoryObjectReturnsTheSameBufferForSmallerSize )
			{
				// INIT
				raw_memory_object rmo;

				// ACT
				double *bufferDoubles1 = rmo.get<double>(100);
				char *bufferChars = rmo.get<char>(100);

				// ASSERT
				assert_equal(reinterpret_cast<void *>(bufferDoubles1), reinterpret_cast<void *>(bufferChars));

				// ACT
				double *bufferDoubles2 = rmo.get<double>(90);

				// ASSERT
				assert_equal(bufferDoubles1, bufferDoubles2);
			}


			test( AddingVectorToAPointShiftsIt )
			{
				// INIT
				const point_r p1 = { -1.202f, 311.9f }, p2 = { 71.2f, -11.39f };
				const point<int> p3 = { -9, 13 }, p4 = { 911, 1978 };
				const vector_r v1 = { 0.7f, 51.7f }, v2 = { 7.7f, 5.7f };
				const agge_vector<int> v3 = { 81, -171 }, v4 = { -7, 19 };

				// ACT / ASSERT
				assert_equal(create_point(-1.202f + 0.7f, 311.9f + 51.7f), p1 + v1);
				assert_equal(create_point(-1.202f + 7.7f, 311.9f + 5.7f), p1 + v2);
				assert_equal(create_point(71.2f + 0.7f, -11.39f + 51.7f), p2 + v1);
				assert_equal(create_point(71.2f + 7.7f, -11.39f + 5.7f), p2 + v2);

				assert_equal(create_point(-9 + 81, 13 + -171), p3 + v3);
				assert_equal(create_point(-9 + -7, 13 + 19), p3 + v4);
				assert_equal(create_point(911 + 81, 1978 + -171), p4 + v3);
				assert_equal(create_point(911 + -7, 1978 + 19), p4 + v4);
			}


			test( PointsSubtractionProducesVector )
			{
				// INIT
				const point_r p1 = { -1.202f, 311.9f }, p2 = { 71.2f, -11.39f }, p3 = { 0.2f, 0.1f };
				const point<int> p4 = { -9, 13 }, p5 = { 911, 1978 }, p6 = { 0, 0 };

				// ACT / ASSERT
				assert_equal(create_vector(-72.402f, 323.29f), p1 - p2);
				assert_equal(create_vector(71.0f, -11.49f), p2 - p3);
				assert_equal(create_vector(1.402f, -311.8f), p3 - p1);

				assert_equal(create_vector(-920, -1965), p4 - p5);
				assert_equal(create_vector(911, 1978), p5 - p6);
				assert_equal(create_vector(9, -13), p6 - p4);
			}


			test( FactoringVectorScalesIt )
			{
				// INIT / ACT / ASSERT
				assert_equal(create_vector(5.1f * 7.1f, 5.1f * -9.19f), 5.1f * create_vector(7.1f, -9.19f));
				assert_equal(create_vector(5.1f * 7.1f, 5.1f * -9.19f), create_vector(7.1f, -9.19f) * 5.1f);
				assert_equal(create_vector(2.3f * -3.2f, 2.3f * 4.19f), 2.3f * create_vector(-3.2f, 4.19f));
				assert_equal(create_vector(2.3f * -3.2f, 2.3f * 4.19f), create_vector(-3.2f, 4.19f) * 2.3f);

				assert_equal(create_vector(605, 1005), 5 * create_vector(121, 201));
				assert_equal(create_vector(-44, 2800), create_vector(-11, 700) * 4);
			}


			test( PathCloseGeneratesExpectedCommands )
			{
				// INIT
				real_t dummy;
				path_close pc;

				// ACT / ASSERT
				assert_equal(path_command_end_poly | path_flag_close, pc.vertex(&dummy, &dummy));
				assert_equal(path_command_stop, pc.vertex(&dummy, &dummy));
				assert_equal(path_command_stop, pc.vertex(&dummy, &dummy));

				// ACT
				pc.rewind(0);

				// ACT / ASSERT
				assert_equal(path_command_end_poly | path_flag_close, pc.vertex(&dummy, &dummy));
			}
		end_test_suite
	}
}
