#include <agge/scanline.h>

#include "helpers.h"
#include "mocks.h"

#include <ut/assert.h>
#include <ut/test.h>
#include <utility>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace
		{
			template <typename T>
			bool enough_capacity(raw_memory_object &rmo, count_t size)
			{
				T *p1 = rmo.get<T>(1); // We have to make this call first.

				return p1 == rmo.get<T>(size);
			}
		}


		begin_test_suite( ScanlineAdapterTests )

			test( ScanlineAllocatesTheNecessaryAmountOfCoversOnConstruction )
			{
				// INIT
				mocks::renderer_adapter<> r1;
				raw_memory_object b1;

				mocks::renderer_adapter<uint16_t> r2;
				raw_memory_object b2;

				// INIT / ACT
				scanline_adapter< mocks::renderer_adapter<> > sl1(r1, b1, 11);
				scanline_adapter< mocks::renderer_adapter<uint16_t> > sl2(r2, b2, 17);

				// ASSERT
				assert_is_true(enough_capacity<uint8_t>(b1, 11 + 4));
				assert_is_true(enough_capacity<uint16_t>(b2, 17 + 4));

				// INIT / ACT
				scanline_adapter< mocks::renderer_adapter<> > sl3(r1, b1, 1311);

				// ASSERT
				assert_is_true(enough_capacity<uint8_t>(b1, 1311 + 4));
			}


			test( CoversArrayIsOnlyBeEnlarged )
			{
				// INIT
				typedef mocks::renderer_adapter<uint8_t> renderer;

				renderer r;
				raw_memory_object b;

				scanline_adapter<renderer> sl1(r, b, 11);

				uint8_t *p1 = b.get<uint8_t>(1);

				// INIT / ACT
				scanline_adapter<renderer> sl2(r, b, 10);

				// ASSERT
				assert_equal(p1, b.get<uint8_t>(10 + 4));
			}


			test( CommitingEmptyScanlineLeadsToAnEmptyRenderCall )
			{
				// INIT
				typedef mocks::renderer_adapter<uint8_t> renderer;

				renderer r;
				raw_memory_object b;
				scanline_adapter<renderer> sl(r, b, 11);

				// ACT
				sl.commit();

				// ASSERT
				mocks::renderer_adapter<>::render_log_entry reference[] = { { 0, vector<uint8_t>() } };

				assert_equal(reference, r.render_log);
			}


			test( ScanlineIsPositionedAtZeroOnCreation )
			{
				// INIT
				typedef mocks::renderer_adapter<uint8_t> renderer;

				renderer r;
				raw_memory_object b;
				scanline_adapter<renderer> sl(r, b, 11);

				// ACT
				sl.add_cell(0, 3);

				// ASSERT
				assert_is_empty(r.render_log);

				// ACT
				sl.add_span(1, 2, 4);
				sl.commit();

				// ASSERT
				uint8_t covers[] = { 3, 4, 4, };
				mocks::renderer_adapter<>::render_log_entry reference[] = { { 0, mkvector(covers) } };

				assert_equal(reference, r.render_log);
			}


			test( AddingCellToCleanScanlineMakesNoCommit )
			{
				// INIT
				typedef mocks::renderer_adapter<uint8_t> renderer;

				renderer r;
				raw_memory_object b;
				scanline_adapter<renderer> sl(r, b, 11);

				// ACT
				sl.add_cell(20001, 3);

				// ASSERT
				mocks::renderer_adapter<>::render_log_entry reference[] = { { 0, vector<uint8_t>() } };

				assert_equal(reference, r.render_log);
			}


			test( AddingSpanToCleanScanlineMakesNoCommit )
			{
				// INIT
				typedef mocks::renderer_adapter<uint8_t> renderer;

				renderer r;
				raw_memory_object b;
				scanline_adapter<renderer> sl(r, b, 20);

				// ACT
				sl.add_span(20001, 20, 3);

				// ASSERT
				mocks::renderer_adapter<>::render_log_entry reference[] = { { 0, vector<uint8_t>() } };

				assert_equal(reference, r.render_log);
			}


			test( AddingAdjacentCellsAndSpansMakesNoCommit )
			{
				// INIT
				typedef mocks::renderer_adapter<uint8_t> renderer;

				renderer r;
				raw_memory_object b;
				scanline_adapter<renderer> sl(r, b, 36);

				sl.add_span(1511, 27, 3);

				r.render_log.clear();

				// ACT
				sl.add_span(1538, 3, 70);
				sl.add_cell(1541, 170);
				sl.add_span(1542, 5, 73);

				// ASSERT
				assert_is_empty(r.render_log);
			}


			test( AddingNonAdjacentCellCommitsState )
			{
				// INIT
				typedef mocks::renderer_adapter<uint8_t> renderer;

				renderer r;
				raw_memory_object b;
				scanline_adapter<renderer> sl(r, b, 50);

				// ACT
				sl.add_span(1531, 7, 3);
				sl.add_span(1538, 3, 70);
				sl.add_cell(1541, 170);
				sl.add_span(1542, 5, 73);

				r.render_log.clear();
				sl.add_cell(1550, 1);

				// ASSERT
				uint8_t covers1[] = { 3, 3, 3, 3, 3, 3, 3, 70, 70, 70, 170, 73, 73, 73, 73, 73, };
				mocks::renderer_adapter<>::render_log_entry reference1[] = { { 1531, mkvector(covers1) } };

				assert_equal(reference1, r.render_log);

				// INIT
				r.render_log.clear();

				// ACT (continue from the last add_cell)
				sl.add_cell(1551, 177);
				sl.add_span(1552, 3, 255);
				sl.add_cell(1550, 33);

				// ASSERT
				uint8_t covers2[] = { 1, 177, 255, 255, 255, };
				mocks::renderer_adapter<>::render_log_entry reference2[] = { { 1550, mkvector(covers2) } };

				assert_equal(reference2, r.render_log);
			}


			test( AddCommitCycleMakesRenditionCalls )
			{
				// INIT
				typedef mocks::renderer_adapter<uint16_t> renderer;

				renderer r;
				raw_memory_object b;
				scanline_adapter<renderer> sl(r, b, 16);

				// ACT
				sl.add_cell(3, 13);
				sl.add_span(4, 2, 17);
				sl.add_cell(6, 11);

				r.render_log.clear();
				sl.commit();

				// ASSERT
				uint16_t covers1[] = { 13, 17, 17, 11, };
				renderer::render_log_entry reference1[] = { { 3, mkvector(covers1) } };

				assert_equal(reference1, r.render_log);

				// ACT
				sl.add_span(471, 2, 17);
				sl.add_cell(473, 110);

				r.render_log.clear();
				sl.commit();

				// ASSERT
				uint16_t covers2[] = { 17, 17, 110, };
				renderer::render_log_entry reference2[] = { { 471, mkvector(covers2) } };

				assert_equal(reference2, r.render_log);
			}


			test( StaleCoverValuesAreAccessibleAndUnchanged )
			{
				// INIT
				typedef mocks::renderer_adapter<uint8_t> renderer;

				renderer r;
				raw_memory_object rmo;
				scanline_adapter<renderer> sl(r, rmo, 8);

				sl.add_cell(0, 13);
				sl.add_span(1, 5, 17);
				sl.add_cell(6, 11);
				sl.add_cell(7, 10);
				sl.commit();
				r.raw_render_log.clear();

				// ACT
				sl.add_span(0, 6, 19);
				sl.commit();

				// ASSERT
				uint8_t reference1[] = { 19, 19, 19, 19, 19, 19, 11, 10, };

				assert_equal(1u, r.raw_render_log.size());
				assert_equal(reference1, vector<uint8_t>(r.raw_render_log[0].first, r.raw_render_log[0].first + 8));

				// ACT
				sl.add_span(0, 5, 23);
				sl.commit();

				// ASSERT
				uint8_t reference2[] = { 23, 23, 23, 23, 23, 19, 11, 10, };

				assert_equal(2u, r.raw_render_log.size());
				assert_equal(reference2, vector<uint8_t>(r.raw_render_log[1].first, r.raw_render_log[1].first + 8));
			}


			test( BeginScanlineDelegatesToSetY )
			{
				// INIT
				typedef mocks::renderer_adapter<uint8_t> renderer;

				renderer r;
				raw_memory_object b;
				scanline_adapter<renderer> sl(r, b, 16);

				// ACT / ASSERT
				r.set_y_result = false;
				assert_is_false(sl.begin(7));
				assert_equal(7, r.current_y);

				r.set_y_result = true;
				assert_is_true(sl.begin(-113317));
				assert_equal(-113317, r.current_y);
			}
		end_test_suite
	}
}
