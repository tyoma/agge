#include <agge/vector_rasterizer.h>

#include <utee/ut/assert.h>
#include <utee/ut/test.h>

#include <iterator>

using namespace std;

namespace agge
{
	template <typename T>
	bool operator ==(const vector<T> &lhs, const pod_vector<T> &rhs)
	{	return lhs.size() == rhs.size() && equal(lhs.begin(), lhs.end(), rhs.begin());	}

	bool operator ==(const vector_rasterizer::cell &lhs, const vector_rasterizer::cell &rhs)
	{	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.cover == rhs.cover && lhs.area == rhs.area;	}

	namespace
	{
		int fp(double value)
		{	return static_cast<int>(vector_rasterizer::_1 * value + (value > 0 ? +0.5 : -0.5));	}

		vector<vector_rasterizer::cell> get_scanline_cells(const vector_rasterizer &r, short y)
		{
			vector_rasterizer::scanline_cells row = r[y];

			return vector<vector_rasterizer::cell>(row.first, row.second);
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
				vr.commit();

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

				assert_equal(469, vr2.width());
				assert_equal(-336, vr2.min_y());
				assert_equal(674, vr2.height());
			}


			test( HorizontalLinesDoNotProduceOutput )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(fp(10.0), fp(15.5), fp(70.0), fp(15.5));
				vr.commit();

				// ASSERT
				assert_is_true(vr.cells().empty());

				// ACT
				vr.line(fp(10.0), fp(13.5), fp(70.0), fp(13.5));
				vr.line(fp(11.3), fp(-17.5), fp(29.0), fp(-17.5));
				vr.line(fp(-10.0), fp(215.5), fp(-70.0), fp(215.5));
				vr.commit();

				// ASSERT
				assert_is_true(vr.cells().empty());
			}


			test( RepeatedCommitDoesNotProduceOutput )
			{
				// INIT
				vector_rasterizer vr;

				vr.line(fp(15.0), fp(10.1), fp(15.0), fp(10.7));
				vr.commit();
				vr.reset();

				// ACT
				vr.commit();

				// ASSERT
				assert_is_empty(vr.cells());
			}


			test( PositiveIntraCellVerticalLinesProducePositiveCover )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(fp(15.0), fp(10.1), fp(15.0), fp(11.0));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 15, 10, 0, 230 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(53.0), fp(9.0), fp(53.0), fp(10.0));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 53, 9, 0, 256 },
				};

				assert_equal(reference2, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(-30000.0), fp(-9.8), fp(-30000.0), fp(-9.5));
				vr.commit();

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
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 15, 10, 0, 230 },
					{ 15, 11, 0, 256 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(53.0), fp(9.0), fp(53.0), fp(13.0));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 53, 9, 0, 256 },
					{ 53, 10, 0, 256 },
					{ 53, 11, 0, 256 },
					{ 53, 12, 0, 256 },
				};

				assert_equal(reference2, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(-30001.0), fp(-2.8), fp(-30001.0), fp(-0.5));
				vr.commit();

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
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 15, 10, 0, -13 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(53.0), fp(9.0), fp(53.0), fp(8.0));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 53, 8, 0, -256 },
				};

				assert_equal(reference2, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(-30000.0), fp(-9.8), fp(-30000.0), fp(-10.0));
				vr.commit();

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
				vr.commit();

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
				vr.commit();

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
				vr.commit();

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
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 15, 10, 36800, 230 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(53.71), fp(9.0), fp(53.71), fp(10.0));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 53, 9, 93184, 256 },
				};

				assert_equal(reference2, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(-3000.99), fp(-119.8), fp(-3000.99), fp(-119.5));
				vr.commit();

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
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 15, 10, 20240, 230 },
					{ 15, 11, 22528, 256 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(53.73), fp(9.0), fp(53.73), fp(13.0));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 53, 9, 95744, 256 },
					{ 53, 10, 95744, 256 },
					{ 53, 11, 95744, 256 },
					{ 53, 12, 95744, 256 },
				};

				assert_equal(reference2, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(-301.91), fp(-2.8), fp(-301.91), fp(-0.5));
				vr.commit();

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
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 15, 10, -676, -13 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(53.172), fp(9.0), fp(53.172), fp(8.0));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 53, 8, -22528, -256 },
				};

				assert_equal(reference2, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(-3000.75), fp(-9.8), fp(-3000.75), fp(-10.0));
				vr.commit();

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
				vr.commit();

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
				vr.commit();

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
				vr.commit();

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
				vr.commit();

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
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 0, 0, (212 + 36) * 74, 74 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT
				vr.line(fp(-3.93), fp(-17000.7), fp(-3.428), fp(-17000.3)); // in fp - [(-1006, -4352179); (-878, -4352077)), v(128, 102)
				vr.commit();

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
				vr.commit();

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
				vr.commit();

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
				vr.commit();

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
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 5, 130, 1100, 10 },
					{ 4, 130, 6400, 25 },
					{ 3, 130, 6144, 24 },
					{ 2, 130, 6400, 25 },
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
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 5, 130, -1210, -11 },
					{ 4, 130, -6400, -25},
					{ 3, 130, -6144, -24 },
					{ 2, 130, -6400, -25 },
					{ 1, 130, -5488, -16 },
				};

				assert_equal(reference1, vr.cells());

				// INIT
				vr.reset();

				// ACT (extra-thin line)
				vr.line(0, 0, 512, -1);
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 0, -1, -256, -1 },
				};

				assert_equal(reference2, vr.cells());
			}


			test( InterCellLowRisingLineRepresentedBySparsedCells )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(0, 0, 1536, 3);
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 1, 0, 256, 1 },
					{ 3, 0, 256, 1 },
					{ 5, 0, 256, 1 },
				};

				assert_equal(reference, vr.cells());
			}


			test( InterCellHLineSpanningFrom0To254 )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(fp(0.0), fp(0.0), fp(255.0), fp(1.0 * 255 / 256));
				vr.commit();

				// ASSERT
				vector<vector_rasterizer::cell> reference;

				for (short x = 0; x < 255; ++x)
				{
					const vector_rasterizer::cell c = { x, 0, 256, 1 };

					reference.push_back(c);
				}

				assert_equal(reference, vr.cells());
			}


			test( InclinedIntraCellLinesCoversAndAreasAreAdditiveOnJoin )
			{
				// INIT
				vector_rasterizer vr;

				// ACT (right triangle)
				vr.line(fp(0.2), fp(0.9), fp(0.2), fp(0.5));
				vr.line(fp(0.2), fp(0.5), fp(2.9), fp(0.7));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 0, 0, -5799, -87 },
					{ 1, 0, 4608, 18 },
					{ 2, 0, 4140, 18 },
				};

				assert_equal(reference, vr.cells());
			}


			test( InterCellPositivelyInclinedTwoShortHLinesProducePositiveCoverAndArea )
			{
				// INIT
				vector_rasterizer vr;

				// ACT (ctg = 252)
				vr.line(fp(1.2), fp(-1.9), fp(1.3), fp(-0.25)); // in fp - [(307, -486); (333, -64)), v(26, 422)
				vr.commit();

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
				vr.commit();

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
				vr.commit();

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
				vr.commit();

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
				vr.commit();

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
				vr.commit();

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
				vr.commit();

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


			test( CellsAreSortedHorizontally )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(0x100000, 0x210, 0x100000, 0x200);
				vr.line(0x1FF000, 0x200, 0x1FF000, 0x210);
				vr.line(0x20F000, 0x203, 0x20F000, 0x220);
				vr.line(0x150000, 0x220, 0x150000, 0x200);
				vr.line(0x20F000, 0x200, 0x20F000, 0x203);
				vr.commit();
				vr.sort();

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
				vr.commit();
				vr.sort();

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 0x1100, 0x3, 0, -0x10 },
					{ 0x1500, 0x3, 0, -0x20 },
					{ 0x2081, 0x3, 0, 0x30 },
				};

				assert_equal(reference2, vr.cells());
			}


			test( CellsAreSortedVertically )
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
				vr.commit();
				vr.sort();

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
				vr.commit();
				vr.sort();

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


			test( EmptyCellsCanBeSorted )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.sort();

				// ASSERT
				assert_is_empty(vr.cells());

				// INIT
				vr.line(0x110000, 0x030230, 0x110000, 0x030200);
				vr.line(0x110010, 0x030200, 0x110010, 0x030230);

				// ACT
				vr.reset();
				vr.sort();

				// ASSERT
				assert_is_empty(vr.cells());
			}


			test( YsAreMoreSignificantThanXsWhileSorting )
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
				vr.commit();
				vr.sort();

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 0x100F, 0x1002, 0x0200, 0 },
					{ 0x700F, 0x1002, 0x0200, 0 },
					{ 0x100F, 0x7002, 0x0200, 0 },
					{ 0x700F, 0x7002, 0x0200, 0 },
				};

				assert_equal(reference, vr.cells());
			}


			test( AccessingSortedCells )
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
				vr.commit();
				vr.sort();

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


			test( UncommittedCellIsCommitedAtSort )
			{
				// INIT
				vector_rasterizer vr;

				// ACT
				vr.line(0x100F00, 0x100210, 0x100F50, 0x100200);
				vr.sort();

				// ASSERT
				const vector_rasterizer::cell reference_row0x1002[] = {
					{ 0x100F, 0x1002, -0x0500, -0x10 },
				};

				assert_equal(reference_row0x1002, get_scanline_cells(vr, 0x1002));
			}


			test( CellsAreNotSortedInitially )
			{
				// INIT
				vector_rasterizer vr;

				// ACT / ASSERT
				assert_is_false(vr.sorted());
			}


			test( CellsSortedPropertyIsSetAfterSort )
			{
				// INIT
				vector_rasterizer vr;

				vr.line(0x100F00, 0x100210, 0x100F50, 0x100200);

				// ACT
				vr.sort();

				// ACT / ASSERT
				assert_is_true(vr.sorted());
			}


			test( SortedPropertyIsSetToUnsortedOnReset )
			{
				// INIT
				vector_rasterizer vr;

				vr.line(0x100F00, 0x100210, 0x100F50, 0x100200);
				vr.sort();

				// ACT
				vr.reset();

				// ACT / ASSERT
				assert_is_false(vr.sorted());
			}
		end_test_suite
	}
}
