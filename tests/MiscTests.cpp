#include <agge/memory.h>

#include <algorithm>

#include <utee/ut/assert.h>
#include <utee/ut/test.h>

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
		end_test_suite
	}
}
