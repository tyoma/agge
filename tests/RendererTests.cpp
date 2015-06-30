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
					: _inprogress(inprogress), _current_y(0x7fffffff)
				{	}

				void begin(int y)
				{
					assert_is_false(_inprogress);

					_inprogress = true;
					_current_y = y;
				}

				void commit()
				{
					assert_is_true(_inprogress);

					_inprogress = false;
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
					assert_not_equal(0, length);

					span s = { _current_y, x, length, cover };

					spans_log.push_back(s);
				}

				vector<span> spans_log;

			private:
				bool _inprogress;
				int _current_y;
			};

			template <size_t precision>
			class raster_source_mockup
			{
			public:
				typedef pair<const cell * /*begin*/, const cell * /*end*/> scanline_cells;
				typedef pair<int, int> range;

				enum { _1_shift = precision };

			public:
				template <typename T, int n>
				raster_source_mockup(const T (&cells)[n], int y0)
					: _vrange(y0, y0 + n - 1)
				{
					for (int i = 0; i != n; ++i)
						_cells.push_back(cells[i]);
				}

				scanline_cells get_scanline_cells(int y) const
				{	return _cells.at(y - _vrange.first);	}

				range vrange() const
				{	return _vrange;	}

			private:
				range _vrange;
				vector<scanline_cells> _cells;
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
				span reference1[] = { { 0x7fffffff, 7, 0, -10 * 512 }, };

				assert_equal(reference1, sl1.spans_log);

				// ACT
				sweep_scanline<8>(begin(cells2), end(cells2), sl2, bypass_alpha());

				// ASSERT
				span reference2[] = { { 0x7fffffff, 1300011, 0, -11 * 512 }, };

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
				cell cells[] = {
					{ 7, 129 * 512, 0 }, { 7, 71 * 512, 0 },
					{ 1911, 199 * 512, 0 }, { 1911, 19 * 512, 0 }, { 1911, -1 * 512, 0 },
				};

				// ACT
				sweep_scanline<8>(begin(cells), end(cells), sl, bypass_alpha());

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
				cell cells[] = {
					{ 7, 0, 10 }, { 7, 0, 3 }, { 10, 0, -10 }, { 10, 0, -3 },
					{ 1911, 0, 17 }, { 1931, 0, -13 }, { 1940, 0, -4 },
				};

				// ACT
				sweep_scanline<8>(begin(cells), end(cells), sl, bypass_alpha());

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
				cell cells1[] = { { 8, 0, 17 }, { 13, 0, -17 }, };
				cell cells2[] = { { 11, 0, 255 }, { 199, 0, -255 }, };
				cell cells3[] = { { 1300011, 0, -100 }, { 1302010, 0, 100 }, };

				// ACT
				sweep_scanline<8>(begin(cells1), end(cells1), sl1, bypass_alpha());

				// ASSERT
				span reference1[] = {
					{ 0x7fffffff, 8, 5, 17 * 512 },
				};

				assert_equal(reference1, sl1.spans_log);

				// ACT
				sweep_scanline<8>(begin(cells2), end(cells2), sl2, bypass_alpha());
				sweep_scanline<8>(begin(cells3), end(cells3), sl2, bypass_alpha());

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
				cell cells1[] = { { -8, 0, 17 }, { 13, 0, -4 }, { 31, 0, -13 }, };
				cell cells2[] = { { 11, 0, 255 }, { 109, 0, -200 }, { 199, 0, -50 }, { 255, 0, -5 }, };
				cell cells3[] = { { 1300011, 0, -100 }, { 1302010, 0, 89 }, { 1302017, 0, 11 }, };

				// ACT
				sweep_scanline<8>(begin(cells1), end(cells1), sl, bypass_alpha());

				// ASSERT
				span reference1[] = {
					{ 0x7fffffff, -8, 21, 17 * 512 },
					{ 0x7fffffff, 13, 18, 13 * 512 },
				};

				assert_equal(reference1, sl.spans_log);

				// ACT
				sweep_scanline<8>(begin(cells2), end(cells2), sl, bypass_alpha());
				sweep_scanline<8>(begin(cells3), end(cells3), sl, bypass_alpha());

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
				cell cells1[] = { { -8, 0, 17 }, { -8, 14 * 512, 0 }, { 13, 0, -4 }, { 13, 11 * 512, 0 }, { 31, 0, -13 }, };
				cell cells2[] = { { 0, 0, 17 }, { 11, 14 * 512, 0 }, { 13, 0, -17 }, };

				// ACT
				sweep_scanline<8>(begin(cells1), end(cells1), sl, bypass_alpha());

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
				sweep_scanline<8>(begin(cells2), end(cells2), sl, bypass_alpha());

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
				cell cells[] = { { 7, 0, 17 }, { 9, 3 * 512, 0 }, { 10, 2 * 512, 0 }, { 12, 0, -17 } };

				// ACT
				sweep_scanline<8>(begin(cells), end(cells), sl, bypass_alpha());

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
				span reference1[] = {
					{ 0x7fffffff, 0, 11, 17 * 256 },
					{ 0x7fffffff, 11, 0, 17 * 256 - 14 * 512 },
					{ 0x7fffffff, 12, 1, 17 * 256 },
				};

				assert_equal(reference1, sl.spans_log);

				// INIT
				sl.spans_log.clear();

				// ACT
				sweep_scanline<10>(begin(cells), end(cells), sl, bypass_alpha());

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
				const cell cells11[] = { { 0, 0, 17 }, { 11, 0, -3 }, { 13, 0, -14 }, };
				raster_source_mockup<8>::scanline_cells cells1[] = { make_pair(begin(cells11), end(cells11)), };
				raster_source_mockup<8> raster1(cells1, 13);

				// ACT
				render(target, raster1, bypass_alpha(), 0, 1);

				// ASSERT
				span reference1[] = {
					{ 13, 0, 11, 17 * 512 },
					{ 13, 11, 2, 14 * 512 },
				};

				assert_equal(reference1, target.spans_log);

				// INIT
				const cell cells21[] = { { 2, 0, 17 }, { 10, 0, -3 }, { 11, 0, -14 }, };
				raster_source_mockup<8>::scanline_cells cells2[] = { make_pair(begin(cells21), end(cells21)), };
				raster_source_mockup<8> raster2(cells2, -131);

				target.spans_log.clear();

				// ACT
				render(target, raster2, bypass_alpha(), 0, 1);

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
				const cell cells11[] = { { 0, 0, 17 }, { 11, 0, -17 }, };
				raster_source_mockup<7>::scanline_cells cells1[] = { make_pair(begin(cells11), end(cells11)), };
				raster_source_mockup<7> raster1(cells1, 13);

				// ACT
				render(target, raster1, bypass_alpha(), 0, 1);

				// ASSERT
				span reference1[] = {
					{ 13, 0, 11, 17 * 256 },
				};

				assert_equal(reference1, target.spans_log);

				// INIT
				const cell cells21[] = { { 2, 0, 17 }, { 10, 0, -17 }, };
				raster_source_mockup<11>::scanline_cells cells2[] = { make_pair(begin(cells21), end(cells21)), };
				raster_source_mockup<11> raster2(cells2, 23);

				target.spans_log.clear();

				// ACT
				render(target, raster2, bypass_alpha(), 0, 1);

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
				const cell cells11[] = { { 0, 0, 17 }, { 11, 0, -3 }, { 13, 0, -14 }, };
				const cell cells12[] = { { -1, 0, 170 }, { 7, 0, -3 }, { 17, 0, -167 }, };
				const cell cells13[] = { { 0, 0, 117 }, { 13, 0, -117 }, };
				raster_source_mockup<8>::scanline_cells cells1[] = {
					make_pair(begin(cells11), end(cells11)),
					make_pair(begin(cells12), end(cells12)),
					make_pair(begin(cells13), end(cells13)),
				};
				raster_source_mockup<8> raster1(cells1, 31);

				// ACT
				render(target, raster1, bypass_alpha(), 0, 1);

				// ASSERT
				span reference1[] = {
					{ 31, 0, 11, 17 * 512 }, { 31, 11, 2, 14 * 512 },
					{ 32, -1, 8, 170 * 512 }, { 32, 7, 10, 167 * 512 },
					{ 33, 0, 13, 117 * 512 },
				};

				assert_equal(reference1, target.spans_log);

				// INIT
				const cell cells21[] = { { 2, 0, 255 }, { 3, 0, -100 }, { 5, 0, -155 }, };
				const cell cells22[] = { { 5, 0, 101 }, { 8, 0, -3 }, { 13, 0, -98 }, };
				raster_source_mockup<8>::scanline_cells cells2[] = {
					make_pair(begin(cells21), end(cells21)),
					make_pair(begin(cells22), end(cells22)),
				};
				raster_source_mockup<8> raster2(cells2, 59);

				target.spans_log.clear();

				// ACT
				render(target, raster2, bypass_alpha(), 0, 1);

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
				const cell cells1[] = { { 0, 0, 17 }, { 11, 0, -3 }, { 13, 0, -14 }, };
				const cell cells2[] = { { -1, 0, 170 }, { 7, 0, -3 }, { 17, 0, -167 }, };
				const cell cells3[] = { { 0, 0, 117 }, { 13, 0, -117 }, };
				const cell cells4[] = { { 2, 0, 255 }, { 3, 0, -100 }, { 5, 0, -155 }, };
				const cell cells5[] = { { 5, 0, 101 }, { 8, 0, -3 }, { 13, 0, -98 }, };
				raster_source_mockup<8>::scanline_cells cells[] = {
					make_pair(begin(cells1), end(cells1)),
					make_pair(begin(cells2), end(cells2)),
					make_pair(begin(cells3), end(cells3)),
					make_pair(begin(cells4), end(cells4)),
					make_pair(begin(cells5), end(cells5)),
				};
				raster_source_mockup<8> raster(cells, 79);

				// ACT
				render(target, raster, bypass_alpha(), 0, 2);

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
				render(target, raster, bypass_alpha(), 0, 3);

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
				const cell cells1[] = { { 0, 0, 17 }, { 11, 0, -3 }, { 13, 0, -14 }, };
				const cell cells2[] = { { -1, 0, 170 }, { 7, 0, -3 }, { 17, 0, -167 }, };
				const cell cells3[] = { { 0, 0, 117 }, { 13, 0, -117 }, };
				const cell cells4[] = { { 2, 0, 255 }, { 3, 0, -100 }, { 5, 0, -155 }, };
				const cell cells5[] = { { 5, 0, 101 }, { 8, 0, -3 }, { 13, 0, -98 }, };
				raster_source_mockup<8>::scanline_cells cells[] = {
					make_pair(begin(cells1), end(cells1)),
					make_pair(begin(cells2), end(cells2)),
					make_pair(begin(cells3), end(cells3)),
					make_pair(begin(cells4), end(cells4)),
					make_pair(begin(cells5), end(cells5)),
				};
				raster_source_mockup<8> raster(cells, 1300);

				// ACT
				render(target, raster, bypass_alpha(), 1, 1);

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
				render(target, raster, bypass_alpha(), 3, 2);

				// ASSERT
				span reference2[] = {
					{ 1303, 2, 1, 255 * 512 }, { 1303, 3, 2, 155 * 512 },
				};

				assert_equal(reference2, target.spans_log);
			}

		end_test_suite
	}
}
