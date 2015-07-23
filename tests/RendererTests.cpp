#include <agge/renderer.h>

#include <agge/types.h>

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

			template <typename PixelT, typename CoverT>
			class mock_blender
			{
			public:
				typedef PixelT pixel;
				typedef CoverT cover_type;

				struct fill_log_entry;

			public:
				void operator ()(PixelT *pixels, unsigned int x, unsigned int y, unsigned int length) const
				{
					fill_log_entry entry = { pixels, x, y, length };

					filling_log.push_back(entry);
				}

				void operator ()(PixelT *pixels, unsigned int x, unsigned int y, unsigned int length, const cover_type *covers) const
				{
					assert_not_equal(0u, length);

					int offset = sizeof(cover_type) * 8;

					for (; length; --length, ++pixels, ++covers)
						*pixels = static_cast<pixel>(static_cast<int>(*covers) + (x << offset) + (y << (offset + 8)));
				}

				mutable vector<fill_log_entry> filling_log;
			};


			template <typename PixelT, typename CoverT>
			struct mock_blender<PixelT, CoverT>::fill_log_entry
			{
				pixel *pixels;
				unsigned int x;
				unsigned int y;
				unsigned int length;

				bool operator ==(const fill_log_entry &rhs) const
				{	return pixels == rhs.pixels && x == rhs.x && y == rhs.y && length == rhs.length;	}
			};


			template <typename PixelT, size_t guard_size = 0>
			class mock_bitmap
			{
			public:
				typedef PixelT pixel;

			public:
				mock_bitmap(unsigned int width, unsigned int height)
					: _width(width), _height(height), data((width + guard_size) * height)
				{	}

				pixel *row_ptr(unsigned int y)
				{	return &data[y * (_width + guard_size)];	}

				unsigned int width() const
				{	return _width;	}

				unsigned int height() const
				{	return _height;	}

			public:
				vector<pixel> data;

			private:
				unsigned int _width, _height;
			};
		}

		begin_test_suite( RendererTests )
			test( RenderingSingleCellsPreservesRenditionPositions )
			{
				// INIT
				scanline_mockup sl1, sl2;
				cell cells1[] = { { 7, 10 * 512, 0 }, }, cells2[] = { { 1300011, 11 * 512, 0 }, };

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
				cell cells[] = { { 7, 129 * 512, 0 }, { 17, 71 * 512, 0 }, { 18, 19 * 512, 0 }, };

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
				cell cells[] = {
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
				cell cells[] = {
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
				cell cells1[] = { { 8, 0, 17 }, { 13, 0, -17 }, };
				cell cells2[] = { { 11, 0, 255 }, { 199, 0, -255 }, };
				cell cells3[] = { { 1300011, 0, -100 }, { 1302010, 0, 100 }, };

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
				cell cells1[] = { { -8, 0, 17 }, { 13, 0, -4 }, { 31, 0, -13 }, };
				cell cells2[] = { { 11, 0, 255 }, { 109, 0, -200 }, { 199, 0, -50 }, { 255, 0, -5 }, };
				cell cells3[] = { { 1300011, 0, -100 }, { 1302010, 0, 89 }, { 1302017, 0, 11 }, };

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
				cell cells1[] = { { -8, 0, 17 }, { -8, 14 * 512, 0 }, { 13, 0, -4 }, { 13, 11 * 512, 0 }, { 31, 0, -13 }, };
				cell cells2[] = { { 0, 0, 17 }, { 11, 14 * 512, 0 }, { 13, 0, -17 }, };

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
				cell cells[] = { { 7, 0, 17 }, { 9, 3 * 512, 0 }, { 10, 2 * 512, 0 }, { 12, 0, -17 } };

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
				cell cells;

				// ACT
				sweep_scanline<8>(sl, &cells, &cells, bypass_alpha());

				// ASSERT
				assert_is_empty(sl.spans_log);
			}


			test( SubpixelPrecisionShiftIsRespected )
			{
				// INIT
				scanline_mockup sl;
				cell cells[] = { { 0, 0, 17 }, { 11, 14 * 512, 0 }, { 13, 0, -17 }, };

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

			test( ScanlineIsOmittedIfCannotBegin )
			{
				// INIT
				scanline_mockup target(false);
				const cell cells1[] = { { 0, 0, 17 }, { 11, 0, -3 }, { 13, 0, -14 }, };
				const cell cells2[] = { { -1, 0, 170 }, { 7, 0, -3 }, { 17, 0, -167 }, };
				const cell cells3[] = { { 0, 0, 117 }, { 13, 0, -117 }, };
				raster_source_mockup<8>::scanline_cells cells[] = {
					make_pair(begin(cells1), end(cells1)),
					make_pair(begin(cells2), end(cells2)),
					make_pair(begin(cells3), end(cells3)),
				};
				raster_source_mockup<8> raster(cells, 31);

				// ACT
				target.excepted_y = 32;
				render(target, raster, bypass_alpha(), 0, 1);

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
				render(target, raster, bypass_alpha(), 0, 1);

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
				mock_blender<int, short> blender1;
				mock_bitmap<int> bitmap1(7, 5);
				renderer::adapter< mock_bitmap<int>, mock_blender<int, short> > r1(bitmap1, blender1);

				uint8_t covers2[] = { 0xED, 0x08, 0x91, };
				mock_blender<uint8_t, uint8_t> blender2;
				mock_bitmap<uint8_t> bitmap2(5, 4);
				renderer::adapter< mock_bitmap<uint8_t>, mock_blender<uint8_t, uint8_t> > r2(bitmap2, blender2);

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
				mock_blender<uint8_t, uint8_t> blender;
				mock_bitmap<uint8_t, 2> bitmap(8, 3);
				renderer::adapter< mock_bitmap<uint8_t, 2>, mock_blender<uint8_t, uint8_t> > r(bitmap, blender);

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
				mock_blender<int, short> blender;
				mock_bitmap<int> bitmap1(10, 1000);
				mock_bitmap<int> bitmap2(10, 123);
				renderer::adapter< mock_bitmap<int>, mock_blender<int, short> > r1(bitmap1, blender);
				renderer::adapter< mock_bitmap<int>, mock_blender<int, short> > r2(bitmap2, blender);

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


			test( BitmapFillInvokesBlenderCopyForAllPixels )
			{
				// INIT
				mock_bitmap<int> bitmap1(3, 5);
				mock_bitmap<int> bitmap2(4, 7);
				mock_blender<int, uint8_t> blender;

				// ACT
				fill(bitmap1, blender);

				// ASSERT
				mock_blender<int, uint8_t>::fill_log_entry reference1[] = {
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
				fill(bitmap2, blender);

				// ASSERT
				mock_blender<int, uint8_t>::fill_log_entry reference2[] = {
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

		end_test_suite
	}
}
