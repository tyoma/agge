#include <agge/vector_rasterizer.h>

#include "helpers.h"

#include <tests/common/helpers.h>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace ut
{
	inline void is_empty(const agge::vector_rasterizer::cells_container& i_container, const LocationInfo &i_location)
	{
		agge::vector_rasterizer::cell empty = { };

		are_equal(1u, i_container.size(), i_location);
		are_equal(empty, i_container[0], i_location);
	}
}

namespace agge
{
	namespace
	{
		int fp(double value)
		{	return static_cast<int>(vector_rasterizer::_1 * value + (value > 0 ? +0.5 : -0.5));	}

		vector<vector_rasterizer::cell> get_scanline_cells(const vector_rasterizer &r, short y)
		{
			vector_rasterizer::scanline_cells row = r[y];

			return vector<vector_rasterizer::cell>(row.first, row.second);
		}

		void rectangle(vector_rasterizer &vr, double x1, double y1, double x2, double y2)
		{
			vr.line(fp(x1), fp(y2), fp(x1), fp(y1));
			vr.line(fp(x1), fp(y1), fp(x2), fp(y1));
			vr.line(fp(x2), fp(y1), fp(x2), fp(y2));
			vr.line(fp(x2), fp(y2), fp(x1), fp(y2));
		}

		void assert_hslope(const vector_rasterizer &vr, int ydelta)
		{
			for (int y = vr.min_y(); y != vr.min_y() + vr.height(); ++y)
			{
				assert_is_true(vr[y].second - vr[y].first >= 2);

				for (vector_rasterizer::const_cells_iterator c = vr[y].first; c != vr[y].second; ++c)
				{
					if (!c->cover & !c->area)
						continue;
					assert_is_true((c->cover > 0) == (ydelta > 0));
					ydelta -= c->cover;
				}
			}
			assert_equal(0, ydelta);
		}
	}

	namespace tests
	{
		begin_test_suite( VectorRasterizerTests )
			test( NewlyCreatedRasterizerIsEmpty )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				assert_equal(0, vr.width());
				assert_equal(0, vr.height());
				assert_is_true(vr.empty());
			}


			test( SubPixelLineExpandsMargins )
			{
				// INIT
				vector_rasterizer vr1, vr2;

				// ACT
				vr1.line(fp(13.0), fp(17.0), fp(13.5), fp(17.7));
				vr2.line(fp(-23.1), fp(-37.0), fp(-24.0), fp(-36.7));

				// ACT / ASSERT
				assert_equal(1, vr1.width());
				assert_equal(17, vr1.min_y());
				assert_equal(1, vr1.height());
				assert_is_false(vr1.empty());

				assert_equal(1, vr2.width());
				assert_equal(-37, vr2.min_y());
				assert_equal(1, vr2.height());
			}


			test( BoundsAreResetOnRequest )
			{
				// INIT
				vector_rasterizer vr;

				vr.line(fp(13.0), fp(17.0), fp(13.5), fp(17.7));

				// ACT
				vr.reset();

				// ACT / ASSERT
				assert_equal(0, vr.width());
				assert_equal(0, vr.height());
			}


			test( UncommittedCellIsDiscardedAtReset )
			{
				// INIT
				vector_rasterizer vr;

				vr.line(fp(13.0), fp(17.0), fp(13.5), fp(17.7));

				// ACT
				vr.reset();

				// ASSERT
				assert_is_empty(vr.cells());
			}


			test( LongerLineExpandsMargins )
			{
				// INIT
				vector_rasterizer vr1, vr2;

				// ACT
				vr1.line(fp(13.0), fp(17.0), fp(-133.5), fp(137.7));
				vr2.line(fp(-253.1), fp(337.0), fp(214.0), fp(-336.0));

				// ACT / ASSERT
				assert_equal(148, vr1.width());
				assert_equal(17, vr1.min_y());
				assert_equal(121, vr1.height());
				assert_is_false(vr1.empty());

				assert_equal(469, vr2.width());
				assert_equal(-336, vr2.min_y());
				assert_equal(674, vr2.height());
			}


			test( HorizontalLinesDoesNothing )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(fp(10.0), fp(15.5), fp(70.0), fp(15.5));

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ },
				};

				assert_equal(reference, vr.cells());

				// ACT
				vr.line(fp(10.0), fp(13.5), fp(70.0), fp(13.5));
				vr.line(fp(11.3), fp(-17.5), fp(29.0), fp(-17.5));
				vr.line(fp(-10.0), fp(215.5), fp(-70.0), fp(215.5));

				// ASSERT
				assert_equal(reference, vr.cells());
			}


			test( PositiveIntraCellVerticalLinesProducePositiveCover )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(fp(15.0), fp(10.1), fp(15.0), fp(11.0));

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 15, 10, 0, 230 },
					{ 15, 11, 0, 0 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(53.0), fp(9.0), fp(53.0), fp(10.0));

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 53, 9, 0, 256 },
					{ 53, 10, 0, 0 },
				};

				assert_equal(reference2, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(-30000.0), fp(-9.8), fp(-30000.0), fp(-9.5));

				// ASSERT
				const vector_rasterizer::cell reference3[] = {
					{ -30000, -10, 0, 77 },
				};

				assert_equal(reference3, vr.cells());
			}


			test( PositiveInterCellsVerticalLinesProducePositiveCover )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(fp(15.0), fp(10.1), fp(15.0), fp(12.0));

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 15, 10, 0, 230 },
					{ 15, 11, 0, 256 },
					{ 15, 12, 0, 0 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(53.0), fp(9.0), fp(53.0), fp(13.0));

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 53, 9, 0, 256 },
					{ 53, 10, 0, 256 },
					{ 53, 11, 0, 256 },
					{ 53, 12, 0, 256 },
					{ 53, 13, 0, 0 },
				};

				assert_equal(reference2, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(-30001.0), fp(-2.8), fp(-30001.0), fp(-0.5));

				// ASSERT
				const vector_rasterizer::cell reference3[] = {
					{ -30001, -3, 0, 205 },
					{ -30001, -2, 0, 256 },
					{ -30001, -1, 0, 128 },
				};

				assert_equal(reference3, vr.cells());
			}


			test( NegativeIntraCellVerticalLinesProduceNegativeCover )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(fp(15.0), fp(10.1), fp(15.0), fp(10.05));

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 15, 10, 0, -13 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(53.0), fp(9.0), fp(53.0), fp(8.0));

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 53, 8, 0, -256 },
				};

				assert_equal(reference2, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(-30000.0), fp(-9.8), fp(-30000.0), fp(-10.0));

				// ASSERT
				const vector_rasterizer::cell reference3[] = {
					{ -30000, -10, 0, -51 },
				};

				assert_equal(reference3, vr.cells());
			}


			test( NegativeInterCellsVerticalLinesProduceNegativeCover )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(fp(3.0), fp(10.1), fp(3.0), fp(5.0));

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 3, 10, 0, -26 },
					{ 3, 9, 0, -256 },
					{ 3, 8, 0, -256 },
					{ 3, 7, 0, -256 },
					{ 3, 6, 0, -256 },
					{ 3, 5, 0, -256 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(23.0), fp(19.0), fp(23.0), fp(17.0));

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 23, 18, 0, -256 },
					{ 23, 17, 0, -256 },
				};

				assert_equal(reference2, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(-30001.0), fp(-2.8), fp(-30001.0), fp(-4.71));

				// ASSERT
				const vector_rasterizer::cell reference3[] = {
					{ -30001, -3, 0, -51 },
					{ -30001, -4, 0, -256 },
					{ -30001, -5, 0, -182 },
				};

				assert_equal(reference3, vr.cells());
			}


			test( PositiveOffsetXIntraCellVerticalLinesProducePositiveCoverAndArea )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(fp(15.312), fp(10.1), fp(15.312), fp(11.0));

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 15, 10, 36800, 230 },
					{ 15, 11, 0, 0 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(53.71), fp(9.0), fp(53.71), fp(10.0));

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 53, 9, 93184, 256 },
					{ 53, 10, 0, 0 },
				};

				assert_equal(reference2, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(-3000.99), fp(-119.8), fp(-3000.99), fp(-119.5));

				// ASSERT
				const vector_rasterizer::cell reference3[] = {
					{ -3001, -120, 462, 77 },
				};

				assert_equal(reference3, vr.cells());
			}


			test( PositiveOffsetXInterCellsVerticalLinesProducePositiveCoverAndArea )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(fp(15.17), fp(10.1), fp(15.17), fp(12.0));

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 15, 10, 20240, 230 },
					{ 15, 11, 22528, 256 },
					{ 15, 12, 0, 0 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(53.73), fp(9.0), fp(53.73), fp(13.0));

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 53, 9, 95744, 256 },
					{ 53, 10, 95744, 256 },
					{ 53, 11, 95744, 256 },
					{ 53, 12, 95744, 256 },
					{ 53, 13, 0, 0 },
				};

				assert_equal(reference2, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(-301.91), fp(-2.8), fp(-301.91), fp(-0.5));

				// ASSERT
				const vector_rasterizer::cell reference3[] = {
					{ -302, -3, 9430, 205 },
					{ -302, -2, 11776, 256 },
					{ -302, -1, 5888, 128 },
				};

				assert_equal(reference3, vr.cells());
			}


			test( NegativeOffsetXIntraCellVerticalLinesProduceNegativeCoverAndArea )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(fp(15.1), fp(10.1), fp(15.1), fp(10.05));

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 15, 10, -676, -13 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(53.172), fp(9.0), fp(53.172), fp(8.0));

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 53, 8, -22528, -256 },
				};

				assert_equal(reference2, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(-3000.75), fp(-9.8), fp(-3000.75), fp(-10.0));

				// ASSERT
				const vector_rasterizer::cell reference3[] = {
					{ -3001, -10, -6528, -51 },
				};

				assert_equal(reference3, vr.cells());
			}


			test( InPixelRectsAreProperlyRasterizedToCells )
			{
				// INIT
				vector_rasterizer vr;

				// ACT (76 x 153)
				vr.line(fp(15.1), fp(10.1), fp(15.4), fp(10.1));
				vr.line(fp(15.4), fp(10.1), fp(15.4), fp(10.7));
				vr.line(fp(15.4), fp(10.7), fp(15.1), fp(10.7));
				vr.line(fp(15.1), fp(10.7), fp(15.1), fp(10.1));

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 15, 10, 2 * 11628, 0 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT (190 x 128)
				vr.line(fp(17.15), fp(101.1), fp(17.15), fp(101.6));
				vr.line(fp(17.15), fp(101.6), fp(17.89), fp(101.6));
				vr.line(fp(17.89), fp(101.6), fp(17.89), fp(101.1));
				vr.line(fp(17.89), fp(101.1), fp(17.15), fp(101.1));

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 17, 101, -2 * 24320, 0 },
				};

				assert_equal(reference2, vr.cells());
			}


			test( TwoRectsInPixelAreProperlyRasterizedToCells )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(fp(15.1), fp(10.1), fp(15.4), fp(10.1));
				vr.line(fp(15.4), fp(10.1), fp(15.4), fp(10.7));
				vr.line(fp(15.4), fp(10.7), fp(15.1), fp(10.7));
				vr.line(fp(15.1), fp(10.7), fp(15.1), fp(10.1));

				vr.line(fp(15.1), fp(10.1), fp(15.2), fp(10.1));
				vr.line(fp(15.2), fp(10.1), fp(15.2), fp(10.2));
				vr.line(fp(15.2), fp(10.2), fp(15.1), fp(10.2));
				vr.line(fp(15.1), fp(10.2), fp(15.1), fp(10.1));

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 15, 10, 24506, 0 },
				};

				assert_equal(reference, vr.cells());
			}


			test( CoverAndAreaAreAddedOnInterCellLineContinuation )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(fp(151.3), fp(11.1), fp(151.3), fp(11.2));
				vr.line(fp(151.3), fp(11.2), fp(151.3), fp(12.7));

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 151, 11, 2 * 77 * 25 + 2 * 77 * 205, 25 + 205 },
					{ 151, 12, 2 * 77 * 179, 179 },
				};

				assert_equal(reference, vr.cells());
			}


			test( IntraCellPositivelyInclinedVectorsProducePositiveCoverAndArea )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(fp(0.83), fp(0.42), fp(0.14), fp(0.71)); // in fp - [(212, 108); (36, 182)), v(-176, 74)

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 0, 0, (212 + 36) * 74, 74 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(-3.93), fp(-17000.7), fp(-3.428), fp(-17000.3)); // in fp - [(-1006, -4352179); (-878, -4352077)), v(128, 102)

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ -4, -17001, (18 + 146) * 102, 102 },
				};

				assert_equal(reference2, vr.cells());
			}


			test( IntraCellLinesCoversAndAreasAreAdditive )
			{
				// INIT
				vector_rasterizer vr;

				// ACT (right triangle)
				vr.line(fp(0.0), fp(0.0), fp(0.5), fp(0.0));	// area: 0, cover: 0
				vr.line(fp(0.5), fp(0.0), fp(0.5), fp(0.5)); // area: 32768, cover: 128
				vr.line(fp(0.5), fp(0.5), fp(0.0), fp(0.0)); // area: -16384, cover: -128

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 0, 0, 16384, 0 },
				};

				assert_equal(reference, vr.cells());
			}


			test( DeltaReminderIsAddedToTheLastCell )
			{
				// INIT
				vector_rasterizer vr;

				// ACT (tg = 2447)
				vr.line(fp(1.34), fp(10.13), fp(2.0), fp(10.523)); // in fp - [(343, 2593); (512, 2694)), v(169, 101)

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 1, 10, 34300, 100 },
					{ 2, 10, 0, 1 },
				};

				assert_equal(reference, vr.cells());
			}


			test( InterCellPositivelyInclinedHLineProducePositiveCoverAndArea )
			{
				// INIT
				vector_rasterizer vr;

				// ACT (forward x, tg = 395)
				vr.line(fp(1.34), fp(130.13), fp(5.43), fp(130.523)); // in fp - [(343, 33313); (1390, 33414)), v(1047, 101)

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 1, 130, 5488, 16 },
					{ 2, 130, 6144, 24 },
					{ 3, 130, 6400, 25 },
					{ 4, 130, 6400, 25 },
					{ 5, 130, 1210, 11 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT (backward x, tg = -395)
				vr.line(fp(5.43), fp(130.13), fp(1.34), fp(130.523)); // in fp - [(1390, 33313); (343, 33414)), v(-1047, 101)

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 5, 130, 1100, 10 },
					{ 4, 130, 6400, 25 },
					{ 3, 130, 6400, 25 },
					{ 2, 130, 6144, 24 },
					{ 1, 130, 5831, 17 },
				};

				assert_equal(reference2, vr.cells());
			}


			test( InterCellNegativelyInclinedHLineProduceNegativeCoverAndArea )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(fp(5.43), fp(130.523), fp(1.34), fp(130.13)); // in fp - [(1390, 33414); (343, 33313)), v(-1047, -101)

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 5, 130, -1210, -11 },
					{ 4, 130, -6400, -25},
					{ 3, 130, -6400, -25 },
					{ 2, 130, -6144, -24 },
					{ 1, 130, -5488, -16 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT (extra-thin line)
				vr.line(0, 0, 512, -1);

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 0, -1, -256, -1 },
					{ 2, -1, 0, 0 },
				};

				assert_equal(reference2, vr.cells());
			}


			test( InterCellIndividualHLineIsRepresentedBySparsedCells )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(0, 0, 1535, 3); // dx = 6 * 256 - 1 (to compensate floating point rounding)

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 1, 0, 256, 1 },
					{ 3, 0, 256, 1 },
					{ 5, 0, 255, 1 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(-256, 256, 2815, 259); // dx = 12 * 256 - 1 (to compensate floating point rounding)

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 2, 1, 256, 1 },
					{ 6, 1, 256, 1 },
					{ 10, 1, 255, 1 },
				};

				assert_equal(reference2, vr.cells());
			}


			test( InterCellHLineSpanningFrom0To254 )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(fp(0.0), fp(0.0), fp(255.0 - 1.0 / 256), fp(1.0 * 255 / 256));

				// ASSERT
				vector_rasterizer::cell reference[255];

				for (short x = 0; x < 254; ++x)
				{
					const vector_rasterizer::cell c = { x, 0, 256, 1 };

					reference[x] = c;
				}

				const vector_rasterizer::cell c = { 254, 0, 255, 1 };

				reference[254] = c;

				assert_equal(reference, vr.cells());
			}


			test( InclinedIntraCellLinesCoversAndAreasAreAdditiveOnJoin )
			{
				// INIT
				vector_rasterizer vr;

				// ACT (right triangle)
				vr.line(fp(0.2), fp(0.9), fp(0.2), fp(0.5));
				vr.line(fp(0.2), fp(0.5), fp(2.9), fp(0.7));

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 0, 0, -5799, -87 },
					{ 1, 0, 4864, 19 },
					{ 2, 0, 3910, 17 },
				};

				assert_equal(reference, vr.cells());
			}


			test( InterCellPositivelyInclinedTwoShortHLinesProducePositiveCoverAndArea )
			{
				// INIT
				vector_rasterizer vr;

				// ACT (ctg = 252)
				vr.line(fp(1.2), fp(-1.9), fp(1.3), fp(-0.25)); // in fp - [(307, -486); (333, -64)), v(26, 422)

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 1, -2, 26680, 230 }, // dx = 14
					{ 1, -1, 27264, 192 }, // dx = 12
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT (ctg = 3319)
				vr.line(fp(0.2), fp(1.9), fp(0.5), fp(2.27)); // in fp - [(51, 486); (128, 581)), v(77, 95)

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 0, 1, 3198, 26 }, // dx = 21
					{ 0, 2, 13800, 69 }, // dx = 56
				};

				assert_equal(reference2, vr.cells());
			}


			test( InterCellNegativelyInclinedTwoShortHLinesProducePositiveCoverAndArea )
			{
				// INIT
				vector_rasterizer vr;

				// ACT (ctg = -713)
				vr.line(fp(1.2), fp(1.9), fp(1.5), fp(0.17)); // in fp - [(307, 486); (384, 44)), v(77, -442)

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 1, 1, -32660, -230 }, // dx = 40
					{ 1, 0, -46428, -212 }, // dx = 37
				};

				assert_equal(reference, vr.cells());
			}


			test( InterCellPositivelyInclinedMultipleShortHLinesProducePositiveCoverAndArea )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(fp(10.2), fp(-1.9), fp(10.9), fp(1.7)); // in fp - [(2611, -486); (2790, 435)), v(179, 921)

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 10, -2, 33580, 230 },
					{ 10, -1, 61440, 256 },
					{ 10, 0, 87040, 256 },
					{ 10, 1, 76075, 179 },
				};

				assert_equal(reference, vr.cells());
			}


			test( InterCellPositivelyInclinedMultipleShortHLinesNearBoundsProducePositiveCoverAndArea )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(fp(8189.2), fp(-1.9), fp(8189.9), fp(1.7));

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 8189, -2, 33580, 230 },
					{ 8189, -1, 61440, 256 },
					{ 8189, 0, 87040, 256 },
					{ 8189, 1, 76075, 179 },
				};

				assert_equal(reference, vr.cells());
			}


			test( InclinedInterCellShortHLinesCoversAndAreasAreAdditiveOnJoin )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(fp(8189.1), fp(-1.7), fp(8189.2), fp(-1.9));
				vr.line(fp(8189.2), fp(-1.9), fp(8189.9), fp(1.7));

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 8189, -2, 29653, 179 },
					{ 8189, -1, 61440, 256 },
					{ 8189, 0, 87040, 256 },
					{ 8189, 1, 76075, 179 },
				};

				assert_equal(reference, vr.cells());
			}


			test( SteepInclinedLineIsSubjectedToSparsedHDeltaCalculation )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(0x1FF005, 0, 0x1FF008, 3 * 3 * 256);

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 0x1FF0, 0, 2560, 256 },
					{ 0x1FF0, 1, 2560, 256 },
					{ 0x1FF0, 2, 2560, 256 },
					{ 0x1FF0, 3, 2816, 256 },
					{ 0x1FF0, 4, 3072, 256 },
					{ 0x1FF0, 5, 3072, 256 },
					{ 0x1FF0, 6, 3328, 256 },
					{ 0x1FF0, 7, 3584, 256 },
					{ 0x1FF0, 8, 3584, 256 },
				};

				assert_equal(reference, vr.cells());
			}


			void CellsAreSortedHorizontallyPre(bool presorted)
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(0x100000, 0x210, 0x100000, 0x200);
				vr.line(0x1FF000, 0x200, 0x1FF000, 0x210);
				vr.line(0x20F000, 0x203, 0x20F000, 0x220);
				vr.line(0x150000, 0x220, 0x150000, 0x200);
				vr.line(0x20F000, 0x200, 0x20F000, 0x203);
				vr.sort(presorted);

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 0x1000, 0x2, 0, -0x10 },
					{ 0x1500, 0x2, 0, -0x20 },
					{ 0x1FF0, 0x2, 0, 0x10 },
					{ 0x20F0, 0x2, 0, 0x1D },
					{ 0x20F0, 0x2, 0, 0x03 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(0x150000, 0x320, 0x150000, 0x300);
				vr.line(0x208100, 0x300, 0x208100, 0x330);
				vr.line(0x110000, 0x330, 0x110000, 0x320);
				vr.sort(presorted);

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 0x1100, 0x3, 0, -0x10 },
					{ 0x1500, 0x3, 0, -0x20 },
					{ 0x2081, 0x3, 0, 0x30 },
				};

				assert_equal(reference2, vr.cells());
			}

			test( CellsAreSortedHorizontally )
			{
				CellsAreSortedHorizontallyPre(true);
				CellsAreSortedHorizontallyPre(false);
			}


			void CellsAreSortedVerticallyPre(bool presorted)
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(0x100000, 0x000210, 0x100000, 0x000200);
				vr.line(0x100010, 0x000200, 0x100010, 0x000210);
				vr.line(0x100000, 0x100210, 0x100000, 0x100200);
				vr.line(0x100010, 0x100200, 0x100010, 0x100210);
				vr.line(0x100000, 0x070110, 0x100000, 0x070100);
				vr.line(0x100010, 0x070100, 0x100010, 0x070110);
				vr.sort(presorted);

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 0x1000, 0x0002, 0x0200, 0 },
					{ 0x1000, 0x0701, 0x0200, 0 },
					{ 0x1000, 0x1002, 0x0200, 0 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(0x110000, 0x030230, 0x110000, 0x030200);
				vr.line(0x110010, 0x030200, 0x110010, 0x030230);
				vr.line(0x110000, 0x100210, 0x110000, 0x100200);
				vr.line(0x110010, 0x100200, 0x110010, 0x100210);
				vr.line(0x110000, 0x070110, 0x110000, 0x070100);
				vr.line(0x110010, 0x070100, 0x110010, 0x070110);
				vr.line(0x110000, 0x070210, 0x110000, 0x070200);
				vr.line(0x110010, 0x070200, 0x110010, 0x070210);
				vr.line(0x110000, 0x030210, 0x110000, 0x030200);
				vr.line(0x110010, 0x030200, 0x110010, 0x030210);
				vr.sort(presorted);

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 0x1100, 0x0302, 0x0600, 0 },
					{ 0x1100, 0x0302, 0x0200, 0 },
					{ 0x1100, 0x0701, 0x0200, 0 },
					{ 0x1100, 0x0702, 0x0200, 0 },
					{ 0x1100, 0x1002, 0x0200, 0 },
				};

				assert_equal(reference2, vr.cells());
			}

			test( CellsAreSortedVertically )
			{
				CellsAreSortedVerticallyPre(true);
				CellsAreSortedVerticallyPre(false);
			}


			void EmptyCellsCanBeSortedPre(bool presorted)
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.sort(presorted);

				// ASSERT
				assert_is_empty(vr.cells());
				assert_is_false(vr.sorted());

				// INIT
				vr.line(0x110000, 0x030230, 0x110000, 0x030200);
				vr.line(0x110010, 0x030200, 0x110010, 0x030230);

				// ACT
				vr.reset();
				vr.sort(presorted);

				// ASSERT
				assert_is_empty(vr.cells());
				assert_is_false(vr.sorted());
			}

			test( EmptyCellsCanBeSorted )
			{
				EmptyCellsCanBeSortedPre(true);
				EmptyCellsCanBeSortedPre(false);
			}


			void NoCellsButNonEmptyRangeIsSortableAndScanlinesOnlyHaveEmptyCellsPre(bool presorted)
			{
				// INIT
				vector_rasterizer vr;

				vr.line(0x12010, 0x23300, 0x12010, 0x23300);
				vr.line(0x12010, 0x23300, 0x12010, 0x23370);
				vr.line(0x12010, 0x23370, 0x12010, 0x23370);
				vr.line(0x12010, 0x23370, 0x12010, 0x23300);

				// ACT
				vr.sort(presorted);

				// ASSERT
				const vector_rasterizer::cell c1 = { 0x120, 0x233, 0, 0 };

				assert_is_true(vr.sorted());
				assert_equal(1, vr[0x233].second - vr[0x233].first);
				assert_equal(c1, *vr[0x233].first);

				// INIT
				vr.line(0x32010, 0x23700, 0x32010, 0x23700);
				vr.line(0x32010, 0x23700, 0x32010, 0x23770);
				vr.line(0x32010, 0x23770, 0x32010, 0x23770);
				vr.line(0x32010, 0x23770, 0x32010, 0x23700);

				// ACT
				vr.sort(presorted);

				// ASSERT
				const vector_rasterizer::cell c2 = { 0x320, 0x237, 0, 0 };

				assert_is_true(vr.sorted());
				assert_equal(vr[0x233].second, vr[0x233].first);
				assert_equal(vr[0x234].second, vr[0x234].first);
				assert_equal(vr[0x235].second, vr[0x235].first);
				assert_equal(vr[0x236].second, vr[0x236].first);
				assert_equal(1, vr[0x237].second - vr[0x237].first);
				assert_equal(c2, *vr[0x233].first);
			}

			test( NoCellsButNonEmptyRangeIsSortableAndScanlinesOnlyHaveEmptyCells )
			{
				NoCellsButNonEmptyRangeIsSortableAndScanlinesOnlyHaveEmptyCellsPre(true);
				NoCellsButNonEmptyRangeIsSortableAndScanlinesOnlyHaveEmptyCellsPre(false);
			}


			void YsAreMoreSignificantThanXsWhileSortingPre(bool presorted)
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(0x100F00, 0x100210, 0x100F00, 0x100200);
				vr.line(0x100F10, 0x100200, 0x100F10, 0x100210);
				vr.line(0x100F00, 0x700210, 0x100F00, 0x700200);
				vr.line(0x100F10, 0x700200, 0x100F10, 0x700210);
				vr.line(0x700F00, 0x700210, 0x700F00, 0x700200);
				vr.line(0x700F10, 0x700200, 0x700F10, 0x700210);
				vr.line(0x700F00, 0x100210, 0x700F00, 0x100200);
				vr.line(0x700F10, 0x100200, 0x700F10, 0x100210);
				vr.sort(presorted);

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 0x100F, 0x1002, 0x0200, 0 },
					{ 0x700F, 0x1002, 0x0200, 0 },
					{ 0x100F, 0x7002, 0x0200, 0 },
					{ 0x700F, 0x7002, 0x0200, 0 },
				};

				assert_equal(reference, vr.cells());
			}

			test( YsAreMoreSignificantThanXsWhileSorting )
			{
				YsAreMoreSignificantThanXsWhileSortingPre(true);
				YsAreMoreSignificantThanXsWhileSortingPre(false);
			}


			void AccessingSortedCellsPre(bool presorted)
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(0x100F00, 0x100210, 0x100F00, 0x100200);
				vr.line(0x100F10, 0x100200, 0x100F10, 0x100210);
				vr.line(0x100F00, 0x700210, 0x100F00, 0x700200);
				vr.line(0x100F10, 0x700200, 0x100F10, 0x700210);
				vr.line(0x700F00, 0x700210, 0x700F00, 0x700200);
				vr.line(0x700F10, 0x700200, 0x700F10, 0x700210);
				vr.line(0x700F00, 0x100210, 0x700F00, 0x100200);
				vr.line(0x700F10, 0x100200, 0x700F10, 0x100210);
				vr.line(0x100F00, 0x100220, 0x100F00, 0x100200);
				vr.line(0x100F10, 0x100200, 0x100F10, 0x100220);
				vr.sort(presorted);

				// ASSERT
				const vector_rasterizer::cell reference1_row0x1002[] = {
					{ 0x100F, 0x1002, 0x0200, 0 },
					{ 0x100F, 0x1002, 0x0400, 0 },
					{ 0x700F, 0x1002, 0x0200, 0 },
				};
				const vector_rasterizer::cell reference1_row0x7002[] = {
					{ 0x100F, 0x7002, 0x0200, 0 },
					{ 0x700F, 0x7002, 0x0200, 0 },
				};

				assert_equal(reference1_row0x1002, get_scanline_cells(vr, 0x1002));
				assert_equal(reference1_row0x7002, get_scanline_cells(vr, 0x7002));
			}

			test( AccessingSortedCells )
			{
				AccessingSortedCellsPre(true);
				AccessingSortedCellsPre(false);
			}


			test( CellsAreNotSortedInitially )
			{
				// INIT
				vector_rasterizer vr;

				// ACT / ASSERT
				assert_is_false(vr.sorted());
			}


			void CellsSortedPropertyIsSetAfterSortPre(bool presorted)
			{
				// INIT
				vector_rasterizer vr;

				vr.line(0x100F00, 0x100210, 0x100F50, 0x100200);

				// ACT
				vr.sort(presorted);

				// ACT / ASSERT
				assert_is_true(vr.sorted());
			}

			test( CellsSortedPropertyIsSetAfterSort )
			{
				CellsSortedPropertyIsSetAfterSortPre(true);
				CellsSortedPropertyIsSetAfterSortPre(false);
			}


			void SortedPropertyIsSetToUnsortedOnResetPre(bool presorted)
			{
				// INIT
				vector_rasterizer vr;

				vr.line(0x100F00, 0x100210, 0x100F50, 0x100200);
				vr.sort(presorted);

				// ACT
				vr.reset();

				// ACT / ASSERT
				assert_is_false(vr.sorted());
			}

			test( SortedPropertyIsSetToUnsortedOnReset )
			{
				SortedPropertyIsSetToUnsortedOnResetPre(true);
				SortedPropertyIsSetToUnsortedOnResetPre(false);
			}


			void SortedPropertyIsResetOnDrawLinePre(bool presorted)
			{
				// INIT
				vector_rasterizer vr;

				vr.line(0x100F00, 0x100210, 0x100F50, 0x100200);
				vr.sort(presorted);

				// ACT
				vr.line(0x100F50, 0x100200, 0x100F50, 0x100100);

				// ACT / ASSERT
				assert_is_false(vr.sorted());
			}

			test( SortedPropertyIsResetOnDrawLine )
			{
				SortedPropertyIsResetOnDrawLinePre(true);
				SortedPropertyIsResetOnDrawLinePre(false);
			}


			void HorizontalLinesDoNotAffectSortedPropertyAndBoxPre(bool presorted)
			{
				// INIT
				vector_rasterizer vr;

				vr.line(0x100F00, 0x100210, 0x100F50, 0x100200);
				vr.sort(presorted);

				// ACT
				vr.line(0x1F50, 0x100200, 0x101F50, 0x100200);
				vr.line(0x1F50, 0x300200, 0x101F50, 0x300200);

				// ACT / ASSERT
				assert_is_true(vr.sorted());
				assert_equal(0x1002, vr.min_y());
				assert_equal(1, vr.height());
				assert_equal(1, vr.width());
			}

			test( HorizontalLinesDoNotAffectSortedPropertyAndBox )
			{
				HorizontalLinesDoNotAffectSortedPropertyAndBoxPre(true);
				HorizontalLinesDoNotAffectSortedPropertyAndBoxPre(false);
			}


			void LongAlmostHorizontalLinesProduceCorrectCellsPre(bool presorted)
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(-0x7FFFFF, -0x2, +0x7FFFFF, +0x2);
				vr.sort(presorted);

				// ASSERT
				assert_equal(2, vr.height());
				assert_hslope(vr, 4);

				// INIT
				vr.reset();

				// ACT
				vr.line(-0x7FFFFF, +0xFF, +0x7FFFFF, -0xFF);
				vr.sort(presorted);

				// ASSERT
				assert_equal(2, vr.height());
				assert_hslope(vr, -0x1FE);

				// INIT
				vr.reset();

				// ACT
				vr.line(+0x7FFFFF, -0xFF, -0x7FFFFF, +0xFF);
				vr.sort(presorted);

				// ASSERT
				assert_equal(2, vr.height());
				assert_hslope(vr, 0x1FE);

				// INIT
				vr.reset();

				// ACT
				vr.line(+0x7FFFFF, +0xFF, -0x7FFFFF, -0xFF);
				vr.sort(presorted);

				// ASSERT
				assert_equal(2, vr.height());
				assert_hslope(vr, -0x1FE);
			}

			test( LongAlmostHorizontalLinesProduceCorrectCells )
			{
				LongAlmostHorizontalLinesProduceCorrectCellsPre(true);
				LongAlmostHorizontalLinesProduceCorrectCellsPre(false);
			}


			test( OperandCellsAreAppendedOnMerge )
			{
				// INIT
				vector_rasterizer vr1, vr2, vr3;

				rectangle(vr1, -0.9, 2.9, 1.9, 1.1);
				rectangle(vr2, 1.3, 1.7, 2.9, 0.1);
				rectangle(vr3, 3.1, -1, 4, -3.75);

				// ACT
				vr1.append(vr2, 0, 0);

				// ASSERT
				vector_rasterizer reference;

				rectangle(reference, -0.9, 2.9, 1.9, 1.1);
				rectangle(reference, 1.3, 1.7, 2.9, 0.1);

				assert_equal(reference.cells(), vr1.cells());

				// ACT
				vr1.append(vr3, 0, 0);

				// ASSERT
				rectangle(reference, 3.1, -1, 4, -3.75);

				assert_equal(reference.cells(), vr1.cells());
			}


			test( OffsetAppendedCellsAreShiftedOnMerge )
			{
				// INIT
				vector_rasterizer vr1, vr2;

				rectangle(vr2, -0.9, 2.9, 1.9, 1.1);

				// ACT
				vr1.append(vr2, 17, 19);

				// ASSERT
				vector_rasterizer reference;

				rectangle(reference, -0.9 + 17, 2.9 + 19, 1.9 + 17, 1.1 + 19);

				assert_equal(reference.cells(), vr1.cells());

				// ACT
				vr1.append(vr2, -13, -71);

				// ASSERT
				rectangle(reference, -0.9 - 13, 2.9 - 71, 1.9 - 13, 1.1 - 71);

				assert_equal(reference.cells(), vr1.cells());
			}


			void RasterizerBecomesUnsortedAfterAppendPre(bool presorted)
			{
				// INIT
				vector_rasterizer vr1, vr2;

				rectangle(vr1, -0.9, 2.9, 1.9, 1.1);
				vr1.sort(presorted);
				rectangle(vr2, -0.9, 2.9, 1.9, 1.1);

				// ACT
				vr1.append(vr2, 1, 2);

				// ASSERT
				assert_is_false(vr1.sorted());
			}

			test( RasterizerBecomesUnsortedAfterAppend )
			{
				RasterizerBecomesUnsortedAfterAppendPre(true);
				RasterizerBecomesUnsortedAfterAppendPre(false);
			}


			test( BoundsAreExtendedOnAppend )
			{
				// INIT
				vector_rasterizer vr1, vr2, vr3;

				rectangle(vr1, 1, 1, 3, -4);
				rectangle(vr2, -10, -131, 13, -100);
				rectangle(vr3, -111, -190, 0, 0);

				// ACT
				vr1.append(vr2, 0, 0);

				// ASSERT
				assert_equal(24, vr1.width());
				assert_equal(-131, vr1.min_y());
				assert_equal(133, vr1.height());

				// ACT
				vr1.append(vr3, 0, 0);

				// ASSERT
				assert_equal(125, vr1.width());
				assert_equal(-190, vr1.min_y());
				assert_equal(192, vr1.height());
			}


			test( BoundsAreExtendedOnAppendWithOffset )
			{
				// INIT
				vector_rasterizer vr1, vr2;

				rectangle(vr1, 1, 1, 3, -4);
				rectangle(vr2, -10, -131, 13, -100);

				// ACT
				vr1.append(vr2, -101, -102);

				// ASSERT
				assert_equal(115, vr1.width());
				assert_equal(-233, vr1.min_y());
				assert_equal(235, vr1.height());

				// ACT
				vr1.append(vr2, 1001, 1003);

				// ASSERT
				assert_equal(1126, vr1.width());
				assert_equal(-233, vr1.min_y());
				assert_equal(1137, vr1.height());
			}


			test( AppendingEmptyDoesNotAffectCurrentRasterizer )
			{
				// INIT
				vector_rasterizer vr, empty;

				rectangle(vr, 1, 1, 3, -4);
				vr.sort();

				pod_vector<vector_rasterizer::cell> cells = vr.cells();

				// ACT
				vr.append(empty, 0, 0);

				// ASSERT
				assert_equal(cells, vr.cells());
				assert_is_true(vr.sorted());
			}


			test( CellsAreSortedAfterCompacting )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				rectangle(vr, 2, 1, 5, 5);
				vr.compact();

				// ASSERT
				vector_rasterizer::cell reference[] = {
					{ 2, 1, 0, -256 }, { 5, 1, 0, 256 },
					{ 2, 2, 0, -256 }, { 5, 2, 0, 256 },
					{ 2, 3, 0, -256 }, { 5, 3, 0, 256 },
					{ 2, 4, 0, -256 }, { 5, 4, 0, 256 },
					{ 5, 5, 0, 0 },
				};

				assert_is_true(vr.sorted());
				assert_equal(reference, vr.cells());
			}


			test( AliasedCellsAreMergedOnCompact )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				rectangle(vr, 2, 1, 7, 5);
				rectangle(vr, 2.75, 3.5, 4, 4);
				rectangle(vr, 7, 2, 7.5, 3);
				vr.compact();

				// ASSERT
				vector_rasterizer::cell reference1[] = {
					{ 2, 1, 0, -256 }, { 7, 1, 0, 256 },
					{ 2, 2, 0, -256 }, { 7, 2, 65536, 256 },
					{ 2, 3, -49152, -384 }, { 4, 3, 0, 128 }, { 7, 3, 0, 256 },
					{ 2, 4, 0, -256 }, { 7, 4, 0, 256 },
				};

				assert_is_true(vr.sorted());
				assert_equal(reference1, vr.cells());
			}


			test( CompactingAnEmptyRasterizerLeavesItUnsortedAndEmpty )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.compact();

				// ASSERT
				assert_is_empty(vr);
			}

		end_test_suite
	}
}
