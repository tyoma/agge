#include <agge/renderer.h>

#include "helpers.h"
#include "mocks.h"

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace
		{
			struct span
			{
				int y;
				int x, length;
				int cover;

				bool operator ==(const span& rhs) const
				{	return y == rhs.y && x == rhs.x && length == rhs.length && cover == rhs.cover;	}
			};


			struct bypass_alpha
			{
				unsigned int operator ()(int area) const
				{	return area;	}
			};


			class scanline_mockup
			{
			public:
				scanline_mockup(bool inprogress = true)
					: excepted_y(-1000000), _inprogress(inprogress), _current_y(0x7fffffff)
				{	}

				bool begin(int y)
				{
					if (y == excepted_y)
						return false;

					assert_is_false(_inprogress);

					_inprogress = true;
					_current_y = y;
					return true;
				}

				void add_cell(int x, int cover)
				{
					assert_is_true(_inprogress);

					span s = { _current_y, x, 0, cover };

					spans_log.push_back(s);
				}

				void add_span(int x, int length, int cover)
				{
					assert_is_true(_inprogress);

					span s = { _current_y, x, length, cover };

					spans_log.push_back(s);
				}

				void commit()
				{
					assert_is_true(_inprogress);

					_inprogress = false;
				}

				int excepted_y;
				vector<span> spans_log;

			private:
				bool _inprogress;
				int _current_y;
			};
		}

		begin_test_suite( RendererTests )
			test( RenderingSingleCellsPreservesRenditionPositions )
			{
				// INIT
				scanline_mockup sl1, sl2;
				mocks::cell cells1[] = { { 7, 10 * 512, 0 }, }, cells2[] = { { 1300011, 11 * 512, 0 }, };

				// ACT
				sweep_scanline<8>(sl1, begin(cells1), end(cells1), bypass_alpha());

				// ASSERT
				span reference1[] = { { 0x7fffffff, 7, 0, -10 * 512 }, };

				assert_equal(reference1, sl1.spans_log);

				// ACT
				sweep_scanline<8>(sl2, begin(cells2), end(cells2), bypass_alpha());

				// ASSERT
				span reference2[] = { { 0x7fffffff, 1300011, 0, -11 * 512 }, };

				assert_equal(reference2, sl2.spans_log);
			}


			test( RenderingCompleteCellsRunProducesCells )
			{
				// INIT
				scanline_mockup sl;
				mocks::cell cells[] = { { 7, 129 * 512, 0 }, { 17, 71 * 512, 0 }, { 18, 19 * 512, 0 }, };

				// ACT
				sweep_scanline<8>(sl, begin(cells), end(cells), bypass_alpha());

				// ASSERT
				span reference[] = {
					{ 0x7fffffff, 7, 0, -129 * 512 },
					{ 0x7fffffff, 17, 0, -71 * 512 },
					{ 0x7fffffff, 18, 0, -19 * 512 },
				};

				assert_equal(reference, sl.spans_log);
			}


			test( AreaIsAccumulatedFromCellsAtTheSamePosition )
			{
				// INIT
				scanline_mockup sl;
				mocks::cell cells[] = {
					{ 7, 129 * 512, 0 }, { 7, 71 * 512, 0 },
					{ 1911, 199 * 512, 0 }, { 1911, 19 * 512, 0 }, { 1911, -1 * 512, 0 },
				};

				// ACT
				sweep_scanline<8>(sl, begin(cells), end(cells), bypass_alpha());

				// ASSERT
				span reference[] = {
					{ 0x7fffffff, 7, 0, -200 * 512 },
					{ 0x7fffffff, 1911, 0, -217 * 512 },
				};

				assert_equal(reference, sl.spans_log);
			}


			test( CoverIsAccumulatedFromCellsAtTheSamePosition )
			{
				// INIT
				scanline_mockup sl;
				mocks::cell cells[] = {
					{ 7, 0, 10 }, { 7, 0, 3 }, { 10, 0, -10 }, { 10, 0, -3 },
					{ 1911, 0, 17 }, { 1931, 0, -13 }, { 1940, 0, -4 },
				};

				// ACT
				sweep_scanline<8>(sl, begin(cells), end(cells), bypass_alpha());

				// ASSERT
				span reference[] = {
					{ 0x7fffffff, 7, 3, 13 * 512 },
					{ 0x7fffffff, 1911, 20, 17 * 512 },
					{ 0x7fffffff, 1931, 9, 4 * 512 },
				};

				assert_equal(reference, sl.spans_log);
			}


			test( RenderingTwoBoundingCellsProducesSpan )
			{
				// INIT
				scanline_mockup sl1, sl2;
				mocks::cell cells1[] = { { 8, 0, 17 }, { 13, 0, -17 }, };
				mocks::cell cells2[] = { { 11, 0, 255 }, { 199, 0, -255 }, };
				mocks::cell cells3[] = { { 1300011, 0, -100 }, { 1302010, 0, 100 }, };

				// ACT
				sweep_scanline<8>(sl1, begin(cells1), end(cells1), bypass_alpha());

				// ASSERT
				span reference1[] = {
					{ 0x7fffffff, 8, 5, 17 * 512 },
				};

				assert_equal(reference1, sl1.spans_log);

				// ACT
				sweep_scanline<8>(sl2, begin(cells2), end(cells2), bypass_alpha());
				sweep_scanline<8>(sl2, begin(cells3), end(cells3), bypass_alpha());

				// ASSERT
				span reference2[] = {
					{ 0x7fffffff, 11, 188, 255 * 512 },
					{ 0x7fffffff, 1300011, 1999, -100 * 512 },
				};

				assert_equal(reference2, sl2.spans_log);
			}


			test( RenderingStagedBoundsProducesCorrespondingNumberOfSpans )
			{
				// INIT
				scanline_mockup sl;
				mocks::cell cells1[] = { { -8, 0, 17 }, { 13, 0, -4 }, { 31, 0, -13 }, };
				mocks::cell cells2[] = { { 11, 0, 255 }, { 109, 0, -200 }, { 199, 0, -50 }, { 255, 0, -5 }, };
				mocks::cell cells3[] = { { 1300011, 0, -100 }, { 1302010, 0, 89 }, { 1302017, 0, 11 }, };

				// ACT
				sweep_scanline<8>(sl, begin(cells1), end(cells1), bypass_alpha());

				// ASSERT
				span reference1[] = {
					{ 0x7fffffff, -8, 21, 17 * 512 },
					{ 0x7fffffff, 13, 18, 13 * 512 },
				};

				assert_equal(reference1, sl.spans_log);

				// ACT
				sweep_scanline<8>(sl, begin(cells2), end(cells2), bypass_alpha());
				sweep_scanline<8>(sl, begin(cells3), end(cells3), bypass_alpha());

				// ASSERT
				span reference2[] = {
					{ 0x7fffffff, -8, 21, 17 * 512 }, { 0x7fffffff, 13, 18, 13 * 512 },
					{ 0x7fffffff, 11, 98, 255 * 512 }, { 0x7fffffff, 109, 90, 55 * 512 }, { 0x7fffffff, 199, 56, 5 * 512 },
					{ 0x7fffffff, 1300011, 1999, -100 * 512 }, { 0x7fffffff, 1302010, 7, -11 * 512 },
				};

				assert_equal(reference2, sl.spans_log);
			}


			test( AreaIsSubtractedFromCoverWhenSweeping )
			{
				// INIT
				scanline_mockup sl;
				mocks::cell cells1[] = { { -8, 0, 17 }, { -8, 14 * 512, 0 }, { 13, 0, -4 }, { 13, 11 * 512, 0 }, { 31, 0, -13 }, };
				mocks::cell cells2[] = { { 0, 0, 17 }, { 11, 14 * 512, 0 }, { 13, 0, -17 }, };

				// ACT
				sweep_scanline<8>(sl, begin(cells1), end(cells1), bypass_alpha());

				// ASSERT
				span reference1[] = {
					{ 0x7fffffff, -8, 0, 3 * 512 },
					{ 0x7fffffff, -7, 20, 17 * 512 },
					{ 0x7fffffff, 13, 0, 2 * 512 },
					{ 0x7fffffff, 14, 17, 13 * 512 },
				};

				assert_equal(reference1, sl.spans_log);

				// INIT
				sl.spans_log.clear();

				// ACT
				sweep_scanline<8>(sl, begin(cells2), end(cells2), bypass_alpha());

				// ASSERT
				span reference2[] = {
					{ 0x7fffffff, 0, 11, 17 * 512 },
					{ 0x7fffffff, 11, 0, 3 * 512 },
					{ 0x7fffffff, 12, 1, 17 * 512 },
				};

				assert_equal(reference2, sl.spans_log);
			}


			test( RenderingTwoNeighborCellsDoesNotProduceInterimEmptySpan )
			{
				// INIT
				scanline_mockup sl;
				mocks::cell cells[] = { { 7, 0, 17 }, { 9, 3 * 512, 0 }, { 10, 2 * 512, 0 }, { 12, 0, -17 } };

				// ACT
				sweep_scanline<8>(sl, begin(cells), end(cells), bypass_alpha());

				// ASSERT
				span reference[] = {
					{ 0x7fffffff, 7, 2, 17 * 512 },
					{ 0x7fffffff, 9, 0, 14 * 512 },
					{ 0x7fffffff, 10, 0, 15 * 512 },
					{ 0x7fffffff, 11, 1, 17 * 512 },
				};

				assert_equal(reference, sl.spans_log);
			}


			test( EmptyRangeProducesNoSpans )
			{
				// INIT
				scanline_mockup sl;
				mocks::cell cells;

				// ACT
				sweep_scanline<8>(sl, &cells, &cells, bypass_alpha());

				// ASSERT
				assert_is_empty(sl.spans_log);
			}


			test( SubpixelPrecisionShiftIsRespected )
			{
				// INIT
				scanline_mockup sl;
				mocks::cell cells[] = { { 0, 0, 17 }, { 11, 14 * 512, 0 }, { 13, 0, -17 }, };

				// ACT
				sweep_scanline<7>(sl, begin(cells), end(cells), bypass_alpha());

				// ASSERT
				span reference1[] = {
					{ 0x7fffffff, 0, 11, 17 * 256 },
					{ 0x7fffffff, 11, 0, 17 * 256 - 14 * 512 },
					{ 0x7fffffff, 12, 1, 17 * 256 },
				};

				assert_equal(reference1, sl.spans_log);

				// INIT
				sl.spans_log.clear();

				// ACT
				sweep_scanline<10>(sl, begin(cells), end(cells), bypass_alpha());

				// ASSERT
				span reference2[] = {
					{ 0x7fffffff, 0, 11, 17 * 2048 },
					{ 0x7fffffff, 11, 0, 17 * 2048 - 14 * 512 },
					{ 0x7fffffff, 12, 1, 17 * 2048 },
				};

				assert_equal(reference2, sl.spans_log);
			}


			test( RenderSingleLineRasterAtProperPosition )
			{
				// INIT
				scanline_mockup target(false);
				const mocks::cell cells11[] = { { 0, 0, 17 }, { 11, 0, -3 }, { 13, 0, -14 }, };
				mocks::mask<8>::scanline_cells cells1[] = { make_pair(begin(cells11), end(cells11)), };
				mocks::mask<8> mask1(cells1, 13);

				// ACT
				render(target, mask1, bypass_alpha(), 0, 1);

				// ASSERT
				span reference1[] = {
					{ 13, 0, 11, 17 * 512 },
					{ 13, 11, 2, 14 * 512 },
				};

				assert_equal(reference1, target.spans_log);

				// INIT
				const mocks::cell cells21[] = { { 2, 0, 17 }, { 10, 0, -3 }, { 11, 0, -14 }, };
				mocks::mask<8>::scanline_cells cells2[] = { make_pair(begin(cells21), end(cells21)), };
				mocks::mask<8> mask2(cells2, -131);

				target.spans_log.clear();

				// ACT
				render(target, mask2, bypass_alpha(), 0, 1);

				// ASSERT
				span reference2[] = {
					{ -131, 2, 8, 17 * 512 },
					{ -131, 10, 1, 14 * 512 },
				};

				assert_equal(reference2, target.spans_log);
			}


			test( SubpixelPrecisionIsRespectedWhileRendering )
			{
				// INIT
				scanline_mockup target(false);
				const mocks::cell cells11[] = { { 0, 0, 17 }, { 11, 0, -17 }, };
				mocks::mask<7>::scanline_cells cells1[] = { make_pair(begin(cells11), end(cells11)), };
				mocks::mask<7> mask1(cells1, 13);

				// ACT
				render(target, mask1, bypass_alpha(), 0, 1);

				// ASSERT
				span reference1[] = {
					{ 13, 0, 11, 17 * 256 },
				};

				assert_equal(reference1, target.spans_log);

				// INIT
				const mocks::cell cells21[] = { { 2, 0, 17 }, { 10, 0, -17 }, };
				mocks::mask<11>::scanline_cells cells2[] = { make_pair(begin(cells21), end(cells21)), };
				mocks::mask<11> mask2(cells2, 23);

				target.spans_log.clear();

				// ACT
				render(target, mask2, bypass_alpha(), 0, 1);

				// ASSERT
				span reference2[] = {
					{ 23, 2, 8, 17 * 4096 },
				};

				assert_equal(reference2, target.spans_log);
			}


			test( RenderMultilinedRasterProgressive )
			{
				// INIT
				scanline_mockup target(false);
				const mocks::cell cells11[] = { { 0, 0, 17 }, { 11, 0, -3 }, { 13, 0, -14 }, };
				const mocks::cell cells12[] = { { -1, 0, 170 }, { 7, 0, -3 }, { 17, 0, -167 }, };
				const mocks::cell cells13[] = { { 0, 0, 117 }, { 13, 0, -117 }, };
				mocks::mask<8>::scanline_cells cells1[] = {
					make_pair(begin(cells11), end(cells11)),
					make_pair(begin(cells12), end(cells12)),
					make_pair(begin(cells13), end(cells13)),
				};
				mocks::mask<8> mask1(cells1, 31);

				// ACT
				render(target, mask1, bypass_alpha(), 0, 1);

				// ASSERT
				span reference1[] = {
					{ 31, 0, 11, 17 * 512 }, { 31, 11, 2, 14 * 512 },
					{ 32, -1, 8, 170 * 512 }, { 32, 7, 10, 167 * 512 },
					{ 33, 0, 13, 117 * 512 },
				};

				assert_equal(reference1, target.spans_log);

				// INIT
				const mocks::cell cells21[] = { { 2, 0, 255 }, { 3, 0, -100 }, { 5, 0, -155 }, };
				const mocks::cell cells22[] = { { 5, 0, 101 }, { 8, 0, -3 }, { 13, 0, -98 }, };
				mocks::mask<8>::scanline_cells cells2[] = {
					make_pair(begin(cells21), end(cells21)),
					make_pair(begin(cells22), end(cells22)),
				};
				mocks::mask<8> mask2(cells2, 59);

				target.spans_log.clear();

				// ACT
				render(target, mask2, bypass_alpha(), 0, 1);

				// ASSERT
				span reference2[] = {
					{ 59, 2, 1, 255 * 512 }, { 59, 3, 2, 155 * 512 },
					{ 60, 5, 3, 101 * 512 }, { 60, 8, 5, 98 * 512 },
				};

				assert_equal(reference2, target.spans_log);
			}


			test( RenderMultilinedRasterInterleaved )
			{
				// INIT
				scanline_mockup target(false);
				const mocks::cell cells1[] = { { 0, 0, 17 }, { 11, 0, -3 }, { 13, 0, -14 }, };
				const mocks::cell cells2[] = { { -1, 0, 170 }, { 7, 0, -3 }, { 17, 0, -167 }, };
				const mocks::cell cells3[] = { { 0, 0, 117 }, { 13, 0, -117 }, };
				const mocks::cell cells4[] = { { 2, 0, 255 }, { 3, 0, -100 }, { 5, 0, -155 }, };
				const mocks::cell cells5[] = { { 5, 0, 101 }, { 8, 0, -3 }, { 13, 0, -98 }, };
				mocks::mask<8>::scanline_cells cells[] = {
					make_pair(begin(cells1), end(cells1)),
					make_pair(begin(cells2), end(cells2)),
					make_pair(begin(cells3), end(cells3)),
					make_pair(begin(cells4), end(cells4)),
					make_pair(begin(cells5), end(cells5)),
				};
				mocks::mask<8> mask(cells, 79);

				// ACT
				render(target, mask, bypass_alpha(), 0, 2);

				// ASSERT
				span reference1[] = {
					{ 79, 0, 11, 17 * 512 }, { 79, 11, 2, 14 * 512 },
					{ 81, 0, 13, 117 * 512 },
					{ 83, 5, 3, 101 * 512 }, { 83, 8, 5, 98 * 512 },
				};

				assert_equal(reference1, target.spans_log);

				// INIT
				target.spans_log.clear();

				// ACT
				render(target, mask, bypass_alpha(), 0, 3);

				// ASSERT
				span reference2[] = {
					{ 79, 0, 11, 17 * 512 }, { 79, 11, 2, 14 * 512 },
					{ 82, 2, 1, 255 * 512 }, { 82, 3, 2, 155 * 512 },
				};

				assert_equal(reference2, target.spans_log);
			}


			test( RenderMultilinedRasterOffset )
			{
				// INIT
				scanline_mockup target(false);
				const mocks::cell cells1[] = { { 0, 0, 17 }, { 11, 0, -3 }, { 13, 0, -14 }, };
				const mocks::cell cells2[] = { { -1, 0, 170 }, { 7, 0, -3 }, { 17, 0, -167 }, };
				const mocks::cell cells3[] = { { 0, 0, 117 }, { 13, 0, -117 }, };
				const mocks::cell cells4[] = { { 2, 0, 255 }, { 3, 0, -100 }, { 5, 0, -155 }, };
				const mocks::cell cells5[] = { { 5, 0, 101 }, { 8, 0, -3 }, { 13, 0, -98 }, };
				mocks::mask<8>::scanline_cells cells[] = {
					make_pair(begin(cells1), end(cells1)),
					make_pair(begin(cells2), end(cells2)),
					make_pair(begin(cells3), end(cells3)),
					make_pair(begin(cells4), end(cells4)),
					make_pair(begin(cells5), end(cells5)),
				};
				mocks::mask<8> mask(cells, 1300);

				// ACT
				render(target, mask, bypass_alpha(), 1, 1);

				// ASSERT
				span reference1[] = {
					{ 1301, -1, 8, 170 * 512 }, { 1301, 7, 10, 167 * 512 },
					{ 1302, 0, 13, 117 * 512 },
					{ 1303, 2, 1, 255 * 512 }, { 1303, 3, 2, 155 * 512 },
					{ 1304, 5, 3, 101 * 512 }, { 1304, 8, 5, 98 * 512 },
				};

				assert_equal(reference1, target.spans_log);

				// INIT
				target.spans_log.clear();

				// ACT
				render(target, mask, bypass_alpha(), 3, 2);

				// ASSERT
				span reference2[] = {
					{ 1303, 2, 1, 255 * 512 }, { 1303, 3, 2, 155 * 512 },
				};

				assert_equal(reference2, target.spans_log);
			}

			test( ScanlineIsOmittedIfCannotBegin )
			{
				// INIT
				scanline_mockup target(false);
				const mocks::cell cells1[] = { { 0, 0, 17 }, { 11, 0, -3 }, { 13, 0, -14 }, };
				const mocks::cell cells2[] = { { -1, 0, 170 }, { 7, 0, -3 }, { 17, 0, -167 }, };
				const mocks::cell cells3[] = { { 0, 0, 117 }, { 13, 0, -117 }, };
				mocks::mask<8>::scanline_cells cells[] = {
					make_pair(begin(cells1), end(cells1)),
					make_pair(begin(cells2), end(cells2)),
					make_pair(begin(cells3), end(cells3)),
				};
				mocks::mask<8> mask(cells, 31);

				// ACT
				target.excepted_y = 32;
				render(target, mask, bypass_alpha(), 0, 1);

				// ASSERT
				span reference1[] = {
					{ 31, 0, 11, 17 * 512 }, { 31, 11, 2, 14 * 512 },
					{ 33, 0, 13, 117 * 512 },
				};

				assert_equal(reference1, target.spans_log);

				// INIT
				target.spans_log.clear();

				// ACT
				target.excepted_y = 33;
				render(target, mask, bypass_alpha(), 0, 1);

				// ASSERT
				span reference2[] = {
					{ 31, 0, 11, 17 * 512 }, { 31, 11, 2, 14 * 512 },
					{ 32, -1, 8, 170 * 512 }, { 32, 7, 10, 167 * 512 },
				};

				assert_equal(reference2, target.spans_log);
			}


			test( RenditionAdapterBlendsAllowedPixelRegionsWithNoLimitations )
			{
				// INIT
				short covers1[] = { 0x1001, 0x0002, 0x4003, 0x00E2, };
				mocks::blender<int, short> blender1;
				mocks::bitmap<int> bitmap1(7, 5);
				renderer::adapter< mocks::bitmap<int>, mocks::blender<int, short> > r1(bitmap1, 0, blender1);

				uint8_t covers2[] = { 0xED, 0x08, 0x91, };
				mocks::blender<uint8_t, uint8_t> blender2;
				mocks::bitmap<uint8_t> bitmap2(5, 4);
				renderer::adapter< mocks::bitmap<uint8_t>, mocks::blender<uint8_t, uint8_t> > r2(bitmap2, 0, blender2);

				// ACT
				r1.set_y(0);
				r1(0, 3, covers1);
				
				r1.set_y(1);
				r1(0, 2, covers1);
				
				r1.set_y(4);
				r1(3, 4, covers1);

				r2.set_y(2);
				r2(4, 1, covers2);
				r2(1, 3, covers2);
				
				r2.set_y(3);
				r2(2, 3, covers2);

				// ASSERT
				int reference1[] = {
					0x00001001, 0x00000002, 0x00004003, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
					0x01001001, 0x01000002, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
					0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
					0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
					0x00000000, 0x00000000, 0x00000000, 0x04031001, 0x04030002, 0x04034003, 0x040300E2,
				};
				uint8_t reference2[] = {
					0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0xED, 0x08, 0x91, 0xED,
					0x00, 0x00, 0xED, 0x08, 0x91,
				};

				assert_equal(reference1, bitmap1.data);
				assert_equal(reference2, bitmap2.data);
			}


			test( RenditionAdapterObeysHorizontalLimits )
			{
				// INIT
				uint8_t covers[] = { 0x51, 0xFF, 0x13, 0x90, 0xE1, };
				mocks::blender<uint8_t, uint8_t> blender;
				mocks::bitmap<uint8_t, 2> bitmap(8, 3);
				renderer::adapter< mocks::bitmap<uint8_t, 2>, mocks::blender<uint8_t, uint8_t> > r(bitmap, 0, blender);

				// ACT
				r.set_y(0);
				r(-2, 3, covers);
				r(5, 5, covers);
				
				r.set_y(1);
				r(-5, 5, covers);
				r(8, 5, covers);
				
				r.set_y(2);
				r(-3, 5, covers);
				r(7, 4, covers);

				// ASSERT
				uint8_t reference[] = {
					0x13, 0x00, 0x00, 0x00, 0x00, 0x51, 0xFF, 0x13, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00,
					0x90, 0xE1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x51, 0x00, 0x00,
				};

				assert_equal(reference, bitmap.data);
			}


			test( SetYRespectsBitmapHeight )
			{
				// INIT
				mocks::blender<int, short> blender;
				mocks::bitmap<int> bitmap1(10, 1000);
				mocks::bitmap<int> bitmap2(10, 123);
				renderer::adapter< mocks::bitmap<int>, mocks::blender<int, short> > r1(bitmap1, 0, blender);
				renderer::adapter< mocks::bitmap<int>, mocks::blender<int, short> > r2(bitmap2, 0, blender);

				// ACT / ASSERT
				assert_is_false(r1.set_y(-1));
				assert_is_true(r1.set_y(0));
				assert_is_true(r1.set_y(1));
				assert_is_true(r1.set_y(999));
				assert_is_false(r1.set_y(1000));

				assert_is_false(r2.set_y(-1));
				assert_is_true(r2.set_y(0));
				assert_is_true(r2.set_y(1));
				assert_is_true(r2.set_y(122));
				assert_is_false(r2.set_y(123));
			}


			test( RendererAdapterShiftsPixelsCoordinatesAccordingToTheWindow )
			{
				// INIT
				uint8_t covers[] = { 0x10, 0x19, 0xF7, 0xE3, 0x79, };
				mocks::blender<int, uint8_t> blender;
				mocks::bitmap<int> bitmap(7, 5);

				typedef renderer::adapter< mocks::bitmap<int>, mocks::blender<int, uint8_t> > renderer_adapter;

				// INIT / ACT
				rect_i window1 = mkrect_sized(-3, -2, 1000, 1000);
				renderer_adapter r1(bitmap, &window1, blender);

				// ACT
				r1.set_y(-1);
				r1(-2, 3, covers);
				
				r1.set_y(1);
				r1(0, 2, covers);
				
				r1.set_y(2);
				r1(1, 3, covers);

				// ASSERT
				int reference1[] = {
					0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
					0x00000000, 0x00FFFE10, 0x00FFFE19, 0x00FFFEF7, 0x00000000, 0x00000000, 0x00000000,
					0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
					0x00000000, 0x00000000, 0x00000000, 0x00010010, 0x00010019, 0x00000000, 0x00000000,
					0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00020110, 0x00020119, 0x000201F7,
				};

				assert_equal(reference1, bitmap.data);

				// INIT
				bitmap.data.assign(bitmap.data.size(), 0);

				// INIT / ACT
				rect_i window2 = mkrect_sized(0x50, 0x30, 1000, 1000);
				renderer_adapter r2(bitmap, &window2, blender);

				// ACT
				r2.set_y(0x30);
				r2(0x50, 4, covers);

				// ASSERT
				int reference2[] = {
					0x00305010, 0x00305019, 0x003050F7, 0x003050E3, 0x00000000, 0x00000000, 0x00000000,
					0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
					0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
					0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
					0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
				};

				assert_equal(reference2, bitmap.data);
			}


			test( RendererAdapterTruncatesShiftedPixelsOnTheNearBound )
			{
				// INIT
				uint8_t covers[] = { 0x10, 0x19, 0xF7, 0xE3, 0x79, };
				mocks::blender<int, uint8_t> blender;
				mocks::bitmap<int> bitmap(5, 2);

				typedef renderer::adapter< mocks::bitmap<int>, mocks::blender<int, uint8_t> > renderer_adapter;

				// INIT / ACT
				rect_i window1 = mkrect_sized(2, 0, 1000, 1000);
				renderer_adapter r1(bitmap, &window1, blender);

				// ACT
				r1.set_y(0);
				r1(0, 5, covers);
				
				r1.set_y(1);
				r1(-1, 5, covers);

				// ASSERT
				int reference1[] = {
					0x000002F7, 0x000002E3, 0x00000279, 0x00000000, 0x00000000,
					0x000102E3, 0x00010279, 0x00000000, 0x00000000, 0x00000000,
				};

				assert_equal(reference1, bitmap.data);

				// INIT
				bitmap.data.assign(bitmap.data.size(), 0);

				// INIT / ACT
				rect_i window2 = mkrect_sized(3, 0, 1000, 1000);
				renderer_adapter r2(bitmap, &window2, blender);

				// ACT
				r2.set_y(0);
				r2(0, 5, covers);

				// ASSERT
				int reference2[] = {
					0x000003E3, 0x00000379, 0x00000000, 0x00000000, 0x00000000,
					0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
				};

				assert_equal(reference2, bitmap.data);
			}


			test( RenditionAdapterWithOffsetObeysHorizontalLimitsOfBitmap )
			{
				// INIT
				uint8_t covers[] = { 0x51, 0xFF, 0x13, 0x90, 0xE1, };
				mocks::blender<uint8_t, uint8_t> blender;
				mocks::bitmap<uint8_t, 3> bitmap(6, 2);
				rect_i window1 = mkrect_sized(-3, 0, 1000, 1000);
				renderer::adapter< mocks::bitmap<uint8_t, 3>, mocks::blender<uint8_t, uint8_t> > r1(bitmap, &window1, blender);
				rect_i window2 = mkrect_sized(-2, 0, 1000, 1000);
				renderer::adapter< mocks::bitmap<uint8_t, 3>, mocks::blender<uint8_t, uint8_t> > r2(bitmap, &window2, blender);

				// ACT
				r1.set_y(0);
				r1(1, 5, covers);
				
				r2.set_y(1);
				r2(3, 2, covers);

				// ASSERT
				uint8_t reference[] = {
					0x00, 0x00, 0x00, 0x00, 0x51, 0xFF, 0x00,	0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x51, 0x00,	0x00, 0x00,
				};

				assert_equal(reference, bitmap.data);
			}


			test( RenditionAdapterWithOffsetObeysHorizontalLimitsOfWindow )
			{
				// INIT
				uint8_t covers[] = { 0x51, 0xFF, 0x13, 0x90, 0xE1, };
				mocks::blender<uint8_t, uint8_t> blender;
				mocks::bitmap<uint8_t, 3> bitmap(6, 2);
				rect_i window1 = mkrect_sized(-3, 0, 5, 1000);
				renderer::adapter< mocks::bitmap<uint8_t, 3>, mocks::blender<uint8_t, uint8_t> > r1(bitmap, &window1, blender);
				rect_i window2 = mkrect_sized(-1, 0, 4, 1000);
				renderer::adapter< mocks::bitmap<uint8_t, 3>, mocks::blender<uint8_t, uint8_t> > r2(bitmap, &window2, blender);

				// ACT
				r1.set_y(0);
				r1(1, 5, covers);
				
				r2.set_y(1);
				r2(0, 5, covers);

				// ASSERT
				uint8_t reference[] = {
					0x00, 0x00, 0x00, 0x00, 0x51, 0x00, 0x00,	0x00, 0x00,
					0x00, 0x51, 0xFF, 0x13, 0x00, 0x00, 0x00,	0x00, 0x00,
				};

				assert_equal(reference, bitmap.data);
			}


			test( RenditionAdapterWithOffsetObeysVerticalLimitsOfBitmap )
			{
				// INIT
				mocks::blender<uint8_t, uint8_t> blender;
				mocks::bitmap<uint8_t> bitmap(6, 11);
				rect_i window1 = mkrect_sized(0, 2, 5, 1000);
				renderer::adapter< mocks::bitmap<uint8_t>, mocks::blender<uint8_t, uint8_t> > r1(bitmap, &window1, blender);
				rect_i window2 = mkrect_sized(0, -3, 4, 1000);
				renderer::adapter< mocks::bitmap<uint8_t>, mocks::blender<uint8_t, uint8_t> > r2(bitmap, &window2, blender);

				// ACT / ASSERT
				assert_is_false(r1.set_y(1));
				assert_is_true(r1.set_y(2));
				assert_is_true(r1.set_y(5));
				assert_is_true(r1.set_y(12));
				assert_is_false(r1.set_y(13));

				assert_is_false(r2.set_y(-4));
				assert_is_true(r2.set_y(-3));
				assert_is_true(r2.set_y(0));
				assert_is_true(r2.set_y(7));
				assert_is_false(r2.set_y(8));
			}


			test( RenditionAdapterWithOffsetObeysVerticalLimitsOfWindow )
			{
				// INIT
				mocks::blender<uint8_t, uint8_t> blender;
				mocks::bitmap<uint8_t> bitmap(6, 110);
				rect_i window1 = mkrect_sized(0, 3, 5, 10);
				renderer::adapter< mocks::bitmap<uint8_t>, mocks::blender<uint8_t, uint8_t> > r1(bitmap, &window1, blender);
				rect_i window2 = mkrect_sized(0, -4, 4, 17);
				renderer::adapter< mocks::bitmap<uint8_t>, mocks::blender<uint8_t, uint8_t> > r2(bitmap, &window2, blender);

				// ACT / ASSERT
				assert_is_false(r1.set_y(2));
				assert_is_true(r1.set_y(3));
				assert_is_true(r1.set_y(5));
				assert_is_true(r1.set_y(12));
				assert_is_false(r1.set_y(13));

				assert_is_false(r2.set_y(-5));
				assert_is_true(r2.set_y(-4));
				assert_is_true(r2.set_y(0));
				assert_is_true(r2.set_y(12));
				assert_is_false(r2.set_y(13));
			}


			test( BitmapFillInvokesBlenderCopyForAllPixels )
			{
				// INIT
				rect_i r1 = { 0, 0, 3, 5 };
				rect_i r2 = { 0, 0, 4, 7 };
				mocks::bitmap<int> bitmap1(3, 5);
				mocks::bitmap<int> bitmap2(4, 7);
				mocks::blender<int, uint8_t> blender;

				// ACT
				fill(bitmap1, r1, blender);

				// ASSERT
				mocks::blender<int, uint8_t>::fill_log_entry reference1[] = {
					{ bitmap1.row_ptr(0), 0, 0, 3 },
					{ bitmap1.row_ptr(1), 0, 1, 3 },
					{ bitmap1.row_ptr(2), 0, 2, 3 },
					{ bitmap1.row_ptr(3), 0, 3, 3 },
					{ bitmap1.row_ptr(4), 0, 4, 3 },
				};

				assert_equal(reference1, blender.filling_log);

				// INIT
				blender.filling_log.clear();

				// ACT
				fill(bitmap2, r2, blender);

				// ASSERT
				mocks::blender<int, uint8_t>::fill_log_entry reference2[] = {
					{ bitmap2.row_ptr(0), 0, 0, 4 },
					{ bitmap2.row_ptr(1), 0, 1, 4 },
					{ bitmap2.row_ptr(2), 0, 2, 4 },
					{ bitmap2.row_ptr(3), 0, 3, 4 },
					{ bitmap2.row_ptr(4), 0, 4, 4 },
					{ bitmap2.row_ptr(5), 0, 5, 4 },
					{ bitmap2.row_ptr(6), 0, 6, 4 },
				};

				assert_equal(reference2, blender.filling_log);
			}


			test( BitmapFillRespectsFillingAreaRect )
			{
				// INIT
				rect_i r1 = { 1, 2, 3, 5 };
				rect_i r2 = { 2, 3, 9, 8 };
				mocks::bitmap<int> bitmap(10, 10);
				mocks::blender<int, uint8_t> blender;

				// ACT
				fill(bitmap, r1, blender);

				// ASSERT
				mocks::blender<int, uint8_t>::fill_log_entry reference1[] = {
					{ bitmap.row_ptr(2), 1, 2, 2 },
					{ bitmap.row_ptr(3), 1, 3, 2 },
					{ bitmap.row_ptr(4), 1, 4, 2 },
				};

				assert_equal(reference1, blender.filling_log);

				// INIT
				blender.filling_log.clear();

				// ACT
				fill(bitmap, r2, blender);

				// ASSERT
				mocks::blender<int, uint8_t>::fill_log_entry reference2[] = {
					{ bitmap.row_ptr(3), 2, 3, 7 },
					{ bitmap.row_ptr(4), 2, 4, 7 },
					{ bitmap.row_ptr(5), 2, 5, 7 },
					{ bitmap.row_ptr(6), 2, 6, 7 },
					{ bitmap.row_ptr(7), 2, 7, 7 },
				};

				assert_equal(reference2, blender.filling_log);
			}


			test( BitmapFillRespsectsClipsFillAreaByBitmapBounds )
			{
				// INIT
				rect_i r = { -1, -1002, 3000, 152 };
				mocks::bitmap<int> bitmap1(5, 6);
				mocks::bitmap<int> bitmap2(15, 7);
				mocks::blender<int, uint8_t> blender;

				// ACT
				fill(bitmap1, r, blender);

				// ASSERT
				mocks::blender<int, uint8_t>::fill_log_entry reference1[] = {
					{ bitmap1.row_ptr(0), 0, 0, 5 },
					{ bitmap1.row_ptr(1), 0, 1, 5 },
					{ bitmap1.row_ptr(2), 0, 2, 5 },
					{ bitmap1.row_ptr(3), 0, 3, 5 },
					{ bitmap1.row_ptr(4), 0, 4, 5 },
					{ bitmap1.row_ptr(5), 0, 5, 5 },
				};

				assert_equal(reference1, blender.filling_log);

				// INIT
				blender.filling_log.clear();

				// ACT
				fill(bitmap2, r, blender);

				// ASSERT
				mocks::blender<int, uint8_t>::fill_log_entry reference2[] = {
					{ bitmap2.row_ptr(0), 0, 0, 15 },
					{ bitmap2.row_ptr(1), 0, 1, 15 },
					{ bitmap2.row_ptr(2), 0, 2, 15 },
					{ bitmap2.row_ptr(3), 0, 3, 15 },
					{ bitmap2.row_ptr(4), 0, 4, 15 },
					{ bitmap2.row_ptr(5), 0, 5, 15 },
					{ bitmap2.row_ptr(6), 0, 6, 15 },
				};

				assert_equal(reference2, blender.filling_log);
			}


			test( BitmapFillDoesNothingOnNonNormalizedRects )
			{
				// INIT
				rect_i r1 = { 3000, -1002, -1, 152 };
				rect_i r2 = { -1, 152, 3000, -1002 };
				mocks::bitmap<int> bitmap(15, 7);
				mocks::blender<int, uint8_t> blender;

				// ACT
				fill(bitmap, r1, blender);
				fill(bitmap, r2, blender);

				// ASSERT
				assert_is_empty(blender.filling_log);
			}


			test( RendererPopulatesBitmapWithMaskData )
			{
				// INIT
				const mocks::cell cells1[] = { { 0, 0, 0x11 }, { 3, 0, -0x03 }, { 7, 0, -0x0E }, };
				const mocks::cell cells2[] = { { 6, 0, 0xAB }, { 9, 0, -0x1E }, { 10, 0, -0x8D }, };
				const mocks::cell cells3[] = { { 1, 0, 0xA0 }, { 9, 0, -0xA0 }, };
				mocks::mask<8>::scanline_cells cells[] = {
					make_pair(begin(cells1), end(cells1)),
					make_pair(begin(cells2), end(cells2)),
					make_pair(begin(cells3), end(cells3)),
				};
				mocks::mask_full<8> mask1(cells, 3);
				mocks::bitmap<uint8_t> bitmap1(11, 7);
				mocks::blender<uint8_t, uint8_t> blender1;

				renderer r;

				// ACT
				r(bitmap1, 0, mask1, blender1, mocks::simple_alpha<uint8_t, 8>());

				// ASSERT
				uint8_t reference1[] = {
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x11, 0x11, 0x11, 0x0E, 0x0E, 0x0E, 0x0E, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAB, 0xAB, 0xAB, 0x8D, 0x00,
					0x00, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				};

				assert_equal(reference1, bitmap1.data);

				// INIT
				mocks::mask_full<8> mask2(cells, 1);
				mocks::bitmap<uint16_t> bitmap2(7, 4);
				mocks::blender<uint16_t, uint8_t> blender2;

				// ACT
				r(bitmap2, 0, mask2, blender2, mocks::simple_alpha<uint8_t, 8>());

				// ASSERT
				uint16_t reference2[] = {
					0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
					0x0011, 0x0011, 0x0011, 0x000E, 0x000E, 0x000E, 0x000E,
					0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x06AB,
					0x0000, 0x01A0, 0x01A0, 0x01A0, 0x01A0, 0x01A0, 0x01A0,
				};

				assert_equal(reference2, bitmap2.data);
			}


			test( RendererPopulatesBitmapWithMaskDataAccordingToWindow )
			{
				// INIT
				const mocks::cell cells1[] = { { 0, 0, 0x11 }, { 3, 0, -0x03 }, { 7, 0, -0x0E }, };
				const mocks::cell cells2[] = { { 6, 0, 0xAB }, { 9, 0, -0x1E }, { 10, 0, -0x8D }, };
				const mocks::cell cells3[] = { { 1, 0, 0xA0 }, { 9, 0, -0xA0 }, };
				const mocks::mask<8>::scanline_cells cells[] = {
					make_pair(begin(cells1), end(cells1)),
					make_pair(begin(cells2), end(cells2)),
					make_pair(begin(cells3), end(cells3)),
				};
				const mocks::mask_full<8> mask1(cells, 3);
				mocks::bitmap<uint8_t> bitmap1(11, 4);
				const mocks::blender<uint8_t, uint8_t> blender1;
				const rect_i window = mkrect_sized(-1, 3, 11, 100);

				renderer r;

				// ACT
				r(bitmap1, &window, mask1, blender1, mocks::simple_alpha<uint8_t, 8>());

				// ASSERT
				uint8_t reference1[] = {
					0x00, 0x11, 0x11, 0x11, 0x0E, 0x0E, 0x0E, 0x0E, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAB, 0xAB, 0xAB, 0x8D,
					0x00, 0x00, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				};

				assert_equal(reference1, bitmap1.data);
			}

		end_test_suite
	}
}
