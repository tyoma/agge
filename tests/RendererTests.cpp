#include <agge/renderer.h>

#include <utee/ut/assert.h>
#include <utee/ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace
		{
			struct cell
			{
				int x;
				int area;
				int cover;
			};

			struct span
			{
				int x, length;
				int cover;

				bool operator ==(const span& rhs) const
				{	return x == rhs.x && length == rhs.length && cover == rhs.cover;	}
			};

			struct bypass_alpha
			{
				unsigned int operator ()(int area) const
				{	return area;	}
			};

			class scanline_mockup
			{
			public:
				void add_cell(int x, int cover)
				{
					span s = { x, 0, cover };

					spans_log.push_back(s);
				}

				void add_span(int x, int length, int cover)
				{
					span s = { x, length, cover };

					spans_log.push_back(s);
				}

				vector<span> spans_log;
			};
		}

		begin_test_suite( RendererTests )
			test( RenderingSingleCellsPreservesRenditionPositions )
			{
				// INIT
				scanline_mockup sl1, sl2;
				cell cells1[] = { { 7, 10 * 512, 0 }, }, cells2[] = { { 1300011, 11 * 512, 0 }, };

				// ACT
				sweep_scanline<8>(begin(cells1), end(cells1), sl1, bypass_alpha());

				// ASSERT
				span reference1[] = { { 7, 0, -10 * 512 }, };

				assert_equal(reference1, sl1.spans_log);

				// ACT
				sweep_scanline<8>(begin(cells2), end(cells2), sl2, bypass_alpha());

				// ASSERT
				span reference2[] = { { 1300011, 0, -11 * 512 }, };

				assert_equal(reference2, sl2.spans_log);
			}


			test( RenderingCompleteCellsRunProducesCells )
			{
				// INIT
				scanline_mockup sl;
				cell cells[] = { { 7, 129 * 512, 0 }, { 17, 71 * 512, 0 }, { 18, 19 * 512, 0 }, };

				// ACT
				sweep_scanline<8>(begin(cells), end(cells), sl, bypass_alpha());

				// ASSERT
				span reference[] = { { 7, 0, -129 * 512 }, { 17, 0, -71 * 512 }, { 18, 0, -19 * 512 }, };

				assert_equal(reference, sl.spans_log);
			}


			test( AreaIsAccumulatedFromCellsAtTheSamePosition )
			{
				// INIT
				scanline_mockup sl;
				cell cells[] = {
					{ 7, 129 * 512, 0 }, { 7, 71 * 512, 0 },
					{ 1911, 199 * 512, 0 }, { 1911, 19 * 512, 0 }, { 1911, -1 * 512, 0 },
				};

				// ACT
				sweep_scanline<8>(begin(cells), end(cells), sl, bypass_alpha());

				// ASSERT
				span reference[] = { { 7, 0, -200 * 512 }, { 1911, 0, -217 * 512 }, };

				assert_equal(reference, sl.spans_log);
			}


			test( CoverIsAccumulatedFromCellsAtTheSamePosition )
			{
				// INIT
				scanline_mockup sl;
				cell cells[] = {
					{ 7, 0, 10 }, { 7, 0, 3 }, { 10, 0, -10 }, { 10, 0, -3 },
					{ 1911, 0, 17 }, { 1931, 0, -13 }, { 1940, 0, -4 },
				};

				// ACT
				sweep_scanline<8>(begin(cells), end(cells), sl, bypass_alpha());

				// ASSERT
				span reference[] = { { 7, 3, 13 * 512 }, { 1911, 20, 17 * 512 }, { 1931, 9, 4 * 512 }, };

				assert_equal(reference, sl.spans_log);
			}


			test( RenderingTwoBoundingCellsProducesSpan )
			{
				// INIT
				scanline_mockup sl1, sl2;
				cell cells1[] = { { 8, 0, 17 }, { 13, 0, -17 }, };
				cell cells2[] = { { 11, 0, 255 }, { 199, 0, -255 }, };
				cell cells3[] = { { 1300011, 0, -100 }, { 1302010, 0, 100 }, };

				// ACT
				sweep_scanline<8>(begin(cells1), end(cells1), sl1, bypass_alpha());

				// ASSERT
				span reference1[] = { { 8, 5, 17 * 512 }, };

				assert_equal(reference1, sl1.spans_log);

				// ACT
				sweep_scanline<8>(begin(cells2), end(cells2), sl2, bypass_alpha());
				sweep_scanline<8>(begin(cells3), end(cells3), sl2, bypass_alpha());

				// ASSERT
				span reference2[] = { { 11, 188, 255 * 512 }, { 1300011, 1999, -100 * 512 }, };

				assert_equal(reference2, sl2.spans_log);
			}


			test( RenderingStagedBoundsProducesCorrespondingNumberOfSpans )
			{
				// INIT
				scanline_mockup sl;
				cell cells1[] = { { -8, 0, 17 }, { 13, 0, -4 }, { 31, 0, -13 }, };
				cell cells2[] = { { 11, 0, 255 }, { 109, 0, -200 }, { 199, 0, -50 }, { 255, 0, -5 }, };
				cell cells3[] = { { 1300011, 0, -100 }, { 1302010, 0, 89 }, { 1302017, 0, 11 }, };

				// ACT
				sweep_scanline<8>(begin(cells1), end(cells1), sl, bypass_alpha());

				// ASSERT
				span reference1[] = { { -8, 21, 17 * 512 }, { 13, 18, 13 * 512 }, };

				assert_equal(reference1, sl.spans_log);

				// ACT
				sweep_scanline<8>(begin(cells2), end(cells2), sl, bypass_alpha());
				sweep_scanline<8>(begin(cells3), end(cells3), sl, bypass_alpha());

				// ASSERT
				span reference2[] = {
					{ -8, 21, 17 * 512 }, { 13, 18, 13 * 512 },
					{ 11, 98, 255 * 512 }, { 109, 90, 55 * 512 }, { 199, 56, 5 * 512 },
					{ 1300011, 1999, -100 * 512 }, { 1302010, 7, -11 * 512 },
				};

				assert_equal(reference2, sl.spans_log);
			}


			test( AreaIsSubtractedFromCoverWhenSweeping )
			{
				// INIT
				scanline_mockup sl;
				cell cells1[] = { { -8, 0, 17 }, { -8, 14 * 512, 0 }, { 13, 0, -4 }, { 13, 11 * 512, 0 }, { 31, 0, -13 }, };
				cell cells2[] = { { 0, 0, 17 }, { 11, 14 * 512, 0 }, { 13, 0, -17 }, };

				// ACT
				sweep_scanline<8>(begin(cells1), end(cells1), sl, bypass_alpha());

				// ASSERT
				span reference1[] = { { -8, 0, 3 * 512 }, { -7, 20, 17 * 512 }, { 13, 0, 2 * 512 }, { 14, 17, 13 * 512 }, };

				assert_equal(reference1, sl.spans_log);

				// INIT
				sl.spans_log.clear();

				// ACT
				sweep_scanline<8>(begin(cells2), end(cells2), sl, bypass_alpha());

				// ASSERT
				span reference2[] = { { 0, 11, 17 * 512 }, { 11, 0, 3 * 512 }, { 12, 1, 17 * 512 }, };

				assert_equal(reference2, sl.spans_log);
			}


			test( EmptyRangeProducesEmptySpans )
			{
				// INIT
				scanline_mockup sl;
				cell cells;

				// ACT
				sweep_scanline<8>(&cells, &cells, sl, bypass_alpha());

				// ASSERT
				assert_is_empty(sl.spans_log);
			}


			test( SubpixelPrecisionShiftIsRespected )
			{
				// INIT
				scanline_mockup sl;
				cell cells[] = { { 0, 0, 17 }, { 11, 14 * 512, 0 }, { 13, 0, -17 }, };

				// ACT
				sweep_scanline<7>(begin(cells), end(cells), sl, bypass_alpha());

				// ASSERT
				span reference1[] = { { 0, 11, 17 * 256 }, { 11, 0, 17 * 256 - 14 * 512 }, { 12, 1, 17 * 256 }, };

				assert_equal(reference1, sl.spans_log);

				// INIT
				sl.spans_log.clear();

				// ACT
				sweep_scanline<10>(begin(cells), end(cells), sl, bypass_alpha());

				// ASSERT
				span reference2[] = { { 0, 11, 17 * 2048 }, { 11, 0, 17 * 2048 - 14 * 512 }, { 12, 1, 17 * 2048 }, };

				assert_equal(reference2, sl.spans_log);
			}

		end_test_suite
	}
}
