#include <agge/scanline.h>

#include "helpers.h"

#include <utee/ut/assert.h>
#include <utee/ut/test.h>
#include <utility>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace
		{
			template <typename CoverT = uint8_t>
			class renderer_mockup
			{
			public:
				typedef CoverT cover_type;

				struct render_log_entry
				{
					int x;
					vector<cover_type> covers;
				};

			public:
				bool set_y(int y)
				{
					current_y = y;
					return set_y_result;
				}

				void operator ()(int x, unsigned int length, const cover_type *covers)
				{
					render_log_entry e = { x, vector<cover_type>(covers, covers + length) };
					render_log.push_back(e);
					raw_render_log.push_back(make_pair(covers, length));
				}

			public:
				int current_y;
				bool set_y_result;
				vector<render_log_entry> render_log;
				vector< pair<const cover_type *, unsigned int> > raw_render_log;
			};

			template <typename CoverT>
			bool operator ==(const struct renderer_mockup<CoverT>::render_log_entry &lhs, const struct renderer_mockup<CoverT>::render_log_entry &rhs)
			{	return lhs.x == rhs.x && lhs.covers == rhs.covers;	}

			template <typename T>
			bool enough_capacity(raw_memory_object &rmo, count_t size)
			{
				T *p1 = rmo.get<T>(1); // We have to make this call first.

				return p1 == rmo.get<T>(size);
			}
		}


		begin_test_suite( ScanlineAdapterTests )

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


			test( ScanlineAllocatesTheNecessaryAmountOfCoversOnConstruction )
			{
				// INIT
				renderer_mockup<> r1;
				raw_memory_object b1;

				renderer_mockup<uint16_t> r2;
				raw_memory_object b2;

				// INIT / ACT
				scanline_adapter< renderer_mockup<> > sl1(r1, b1, 11);
				scanline_adapter< renderer_mockup<uint16_t> > sl2(r2, b2, 17);

				// ASSERT
				assert_is_true(enough_capacity<uint8_t>(b1, 11 + 16));
				assert_is_true(enough_capacity<uint16_t>(b2, 17 + 16));

				// INIT / ACT
				scanline_adapter< renderer_mockup<> > sl3(r1, b1, 1311);

				// ASSERT
				assert_is_true(enough_capacity<uint8_t>(b1, 1311 + 16));
			}


			test( CoversArrayIsOnlyBeEnlarged )
			{
				// INIT
				typedef renderer_mockup<uint8_t> renderer;

				renderer r;
				raw_memory_object b;

				scanline_adapter<renderer> sl1(r, b, 11);

				uint8_t *p1 = b.get<uint8_t>(1);

				// INIT / ACT
				scanline_adapter<renderer> sl2(r, b, 10);

				// ASSERT
				assert_equal(p1, b.get<uint8_t>(10 + 16));
			}


			test( CommitingEmptyScanlineLeadsToAnEmptyRenderCall )
			{
				// INIT
				typedef renderer_mockup<uint8_t> renderer;

				renderer r;
				raw_memory_object b;
				scanline_adapter<renderer> sl(r, b, 11);

				// ACT
				sl.commit();

				// ASSERT
				renderer_mockup<>::render_log_entry reference[] = { { 0, vector<uint8_t>() } };

				assert_equal(reference, r.render_log);
			}


			test( ScanlineIsPositionedAtZeroOnCreation )
			{
				// INIT
				typedef renderer_mockup<uint8_t> renderer;

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
				renderer_mockup<>::render_log_entry reference[] = { { 0, mkvector(covers) } };

				assert_equal(reference, r.render_log);
			}


			test( AddingCellToCleanScanlineMakesNoCommit )
			{
				// INIT
				typedef renderer_mockup<uint8_t> renderer;

				renderer r;
				raw_memory_object b;
				scanline_adapter<renderer> sl(r, b, 11);

				// ACT
				sl.add_cell(20001, 3);

				// ASSERT
				renderer_mockup<>::render_log_entry reference[] = { { 0, vector<uint8_t>() } };

				assert_equal(reference, r.render_log);
			}


			test( AddingSpanToCleanScanlineMakesNoCommit )
			{
				// INIT
				typedef renderer_mockup<uint8_t> renderer;

				renderer r;
				raw_memory_object b;
				scanline_adapter<renderer> sl(r, b, 11);

				// ACT
				sl.add_span(20001, 20, 3);

				// ASSERT
				renderer_mockup<>::render_log_entry reference[] = { { 0, vector<uint8_t>() } };

				assert_equal(reference, r.render_log);
			}


			test( AddingAdjacentCellsAndSpansMakesNoCommit )
			{
				// INIT
				typedef renderer_mockup<uint8_t> renderer;

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
				typedef renderer_mockup<uint8_t> renderer;

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
				renderer_mockup<>::render_log_entry reference1[] = { { 1531, mkvector(covers1) } };

				assert_equal(reference1, r.render_log);

				// INIT
				r.render_log.clear();

				// ACT (continue from the last add_cell)
				sl.add_cell(1551, 177);
				sl.add_span(1552, 3, 255);
				sl.add_cell(1550, 33);

				// ASSERT
				uint8_t covers2[] = { 1, 177, 255, 255, 255, };
				renderer_mockup<>::render_log_entry reference2[] = { { 1550, mkvector(covers2) } };

				assert_equal(reference2, r.render_log);
			}


			test( AddCommitCycleMakesRenditionCalls )
			{
				// INIT
				typedef renderer_mockup<uint16_t> renderer;

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


			test( CoversBufferIsPaddedInTheBegining )
			{
				// INIT
				typedef renderer_mockup<uint8_t> renderer;

				renderer r;
				raw_memory_object rmo;
				scanline_adapter<renderer> sl(r, rmo, 16);
				uint8_t *b = rmo.get<uint8_t>(1);

				// ACT
				sl.add_span(4, 7, 17);
				sl.commit();

				// ASSERT
				assert_equal(&b[4], r.raw_render_log[0].first);
				assert_equal(&b[4], r.raw_render_log[1].first);
				assert_equal(0, b[0]);
				assert_equal(0, b[1]);
				assert_equal(0, b[2]);

				const uint8_t *end = r.raw_render_log[1].first + r.raw_render_log[1].second;

				assert_equal(0, *end++);
				assert_equal(0, *end++);
				assert_equal(0, *end++);

				// ACT
				sl.add_span(4, 2, 3);
				r.raw_render_log.clear();
				sl.commit();

				// ASSERT
				assert_equal(&b[4], r.raw_render_log[0].first);
				assert_equal(0, b[0]);
				assert_equal(0, b[1]);
				assert_equal(0, b[2]);

				end = r.raw_render_log[0].first + r.raw_render_log[0].second;

				assert_equal(0, *end++);
				assert_equal(0, *end++);
				assert_equal(0, *end++);
			}


			test( BeginScanlineDelegatesToSetY )
			{
				// INIT
				typedef renderer_mockup<uint8_t> renderer;

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
