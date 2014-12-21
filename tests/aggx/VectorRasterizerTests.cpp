#include <aggx2/vector_rasterizer.h>

#include <utee/ut/assert.h>
#include <utee/ut/test.h>

#include <iterator>

using namespace std;


namespace ut
{
	template <typename T1, size_t n, typename T2>
	inline void are_equal(T1 (&lhs)[n], const T2 &rhs, const LocationInfo &location)
	{	are_equal(std::vector<typename T2::value_type>(lhs, lhs + n), rhs, location);	}
}


namespace aggx
{
	bool operator ==(const aggx::vector_rasterizer::cell &lhs, const aggx::vector_rasterizer::cell &rhs)
	{	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.cover == rhs.cover && lhs.area == rhs.area;	}

	namespace
	{
		int fp(double value)
		{	return static_cast<int>(vector_rasterizer::_1 * value + (value > 0 ? +0.5 : -0.5));	}
	}

	namespace tests
	{
		begin_test_suite( VectorRasterizerTests )
			test( NewlyCreatedRasterizerIsEmpty )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

				// ACT
				vector_rasterizer::range vrange = vr.vrange();
				vector_rasterizer::range hrange = vr.hrange();

				// ASSERT
				assert_equal(0x7FFF, vrange.first);
				assert_equal(-0x7FFF, vrange.second);
				assert_equal(0x7FFF, hrange.first);
				assert_equal(-0x7FFF, hrange.second);
			}


			test( SubPixelLineExpandsMargins )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr1(cells), vr2(cells);

				// ACT
				vr1.line(fp(13.0), fp(17.0), fp(13.5), fp(17.7));
				vr2.line(fp(-23.1), fp(-37.0), fp(-24.0), fp(-36.7));
				vector_rasterizer::range hrange1 = vr1.hrange();
				vector_rasterizer::range vrange1 = vr1.vrange();
				vector_rasterizer::range hrange2 = vr2.hrange();
				vector_rasterizer::range vrange2 = vr2.vrange();

				// ASSERT
				assert_equal(13, hrange1.first);
				assert_equal(13, hrange1.second);
				assert_equal(17, vrange1.first);
				assert_equal(17, vrange1.second);
				assert_equal(-24, hrange2.first);
				assert_equal(-24, hrange2.second);
				assert_equal(-37, vrange2.first);
				assert_equal(-37, vrange2.second);
			}


			test( BoundsAreResetOnRequest )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

				vr.line(fp(13.0), fp(17.0), fp(13.5), fp(17.7));

				// ACT
				vr.reset();
				vector_rasterizer::range vrange = vr.vrange();
				vector_rasterizer::range hrange = vr.hrange();

				// ASSERT
				assert_equal(0x7FFF, vrange.first);
				assert_equal(-0x7FFF, vrange.second);
				assert_equal(0x7FFF, hrange.first);
				assert_equal(-0x7FFF, hrange.second);
			}


			test( LongerLineExpandsMargins )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr1(cells), vr2(cells);

				// ACT
				vr1.line(fp(13.0), fp(17.0), fp(-133.5), fp(137.7));
				vr2.line(fp(-253.1), fp(337.0), fp(214.0), fp(-336.0));
				vector_rasterizer::range hrange1 = vr1.hrange();
				vector_rasterizer::range vrange1 = vr1.vrange();
				vector_rasterizer::range hrange2 = vr2.hrange();
				vector_rasterizer::range vrange2 = vr2.vrange();

				// ASSERT
				assert_equal(-134, hrange1.first);
				assert_equal(13, hrange1.second);
				assert_equal(17, vrange1.first);
				assert_equal(137, vrange1.second);
				assert_equal(-254, hrange2.first);
				assert_equal(214, hrange2.second);
				assert_equal(-336, vrange2.first);
				assert_equal(337, vrange2.second);
			}


			test( HorizontalLinesDoNotProduceOutput )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

				// ACT
				vr.line(fp(10.0), fp(15.5), fp(70.0), fp(15.5));
				vr.commit();

				// ASSERT
				assert_is_true(cells.empty());

				// ACT
				vr.line(fp(10.0), fp(13.5), fp(70.0), fp(13.5));
				vr.line(fp(11.3), fp(-17.5), fp(29.0), fp(-17.5));
				vr.line(fp(-10.0), fp(215.5), fp(-70.0), fp(215.5));
				vr.commit();

				// ASSERT
				assert_is_true(cells.empty());
			}


			test( RepeatedCommitDoesNotProduceOutput )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

				vr.line(fp(15.0), fp(10.1), fp(15.0), fp(10.7));
				vr.commit();
				cells.clear();

				// ACT
				vr.commit();

				// ASSERT
				assert_is_empty(cells);
			}


			test( PositiveIntraCellVerticalLinesProducePositiveCover )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

				// ACT
				vr.line(fp(15.0), fp(10.1), fp(15.0), fp(11.0));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 15, 10, 0, 230 },
				};

				assert_equal(reference1, cells);

				// INIT
				cells.clear();

				// ACT
				vr.line(fp(53.0), fp(9.0), fp(53.0), fp(10.0));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 53, 9, 0, 256 },
				};

				assert_equal(reference2, cells);

				// INIT
				cells.clear();

				// ACT
				vr.line(fp(-30000.0), fp(-9.8), fp(-30000.0), fp(-9.5));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference3[] = {
					{ -30000, -10, 0, 77 },
				};

				assert_equal(reference3, cells);
			}


			test( PositiveInterCellsVerticalLinesProducePositiveCover )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

				// ACT
				vr.line(fp(15.0), fp(10.1), fp(15.0), fp(12.0));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 15, 10, 0, 230 },
					{ 15, 11, 0, 256 },
				};

				assert_equal(reference1, cells);

				// INIT
				cells.clear();

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

				assert_equal(reference2, cells);

				// INIT
				cells.clear();

				// ACT
				vr.line(fp(-30001.0), fp(-2.8), fp(-30001.0), fp(-0.5));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference3[] = {
					{ -30001, -3, 0, 205 },
					{ -30001, -2, 0, 256 },
					{ -30001, -1, 0, 128 },
				};

				assert_equal(reference3, cells);
			}


			test( NegativeIntraCellVerticalLinesProduceNegativeCover )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

				// ACT
				vr.line(fp(15.0), fp(10.1), fp(15.0), fp(10.05));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 15, 10, 0, -13 },
				};

				assert_equal(reference1, cells);

				// INIT
				cells.clear();

				// ACT
				vr.line(fp(53.0), fp(9.0), fp(53.0), fp(8.0));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 53, 8, 0, -256 },
				};

				assert_equal(reference2, cells);

				// INIT
				cells.clear();

				// ACT
				vr.line(fp(-30000.0), fp(-9.8), fp(-30000.0), fp(-10.0));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference3[] = {
					{ -30000, -10, 0, -51 },
				};

				assert_equal(reference3, cells);
			}


			test( NegativeInterCellsVerticalLinesProduceNegativeCover )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

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

				assert_equal(reference1, cells);

				// INIT
				cells.clear();

				// ACT
				vr.line(fp(23.0), fp(19.0), fp(23.0), fp(17.0));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 23, 18, 0, -256 },
					{ 23, 17, 0, -256 },
				};

				assert_equal(reference2, cells);

				// INIT
				cells.clear();

				// ACT
				vr.line(fp(-30001.0), fp(-2.8), fp(-30001.0), fp(-4.71));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference3[] = {
					{ -30001, -3, 0, -51 },
					{ -30001, -4, 0, -256 },
					{ -30001, -5, 0, -182 },
				};

				assert_equal(reference3, cells);
			}


			test( PositiveOffsetXIntraCellVerticalLinesProducePositiveCoverAndArea )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

				// ACT
				vr.line(fp(15.312), fp(10.1), fp(15.312), fp(11.0));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 15, 10, 36800, 230 },
				};

				assert_equal(reference1, cells);

				// INIT
				cells.clear();

				// ACT
				vr.line(fp(53.71), fp(9.0), fp(53.71), fp(10.0));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 53, 9, 93184, 256 },
				};

				assert_equal(reference2, cells);

				// INIT
				cells.clear();

				// ACT
				vr.line(fp(-3000.99), fp(-119.8), fp(-3000.99), fp(-119.5));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference3[] = {
					{ -3001, -120, 462, 77 },
				};

				assert_equal(reference3, cells);
			}


			test( PositiveOffsetXInterCellsVerticalLinesProducePositiveCoverAndArea )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

				// ACT
				vr.line(fp(15.17), fp(10.1), fp(15.17), fp(12.0));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 15, 10, 20240, 230 },
					{ 15, 11, 22528, 256 },
				};

				assert_equal(reference1, cells);

				// INIT
				cells.clear();

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

				assert_equal(reference2, cells);

				// INIT
				cells.clear();

				// ACT
				vr.line(fp(-301.91), fp(-2.8), fp(-301.91), fp(-0.5));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference3[] = {
					{ -302, -3, 9430, 205 },
					{ -302, -2, 11776, 256 },
					{ -302, -1, 5888, 128 },
				};

				assert_equal(reference3, cells);
			}


			test( NegativeOffsetXIntraCellVerticalLinesProduceNegativeCoverAndArea )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

				// ACT
				vr.line(fp(15.1), fp(10.1), fp(15.1), fp(10.05));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 15, 10, -676, -13 },
				};

				assert_equal(reference1, cells);

				// INIT
				cells.clear();

				// ACT
				vr.line(fp(53.172), fp(9.0), fp(53.172), fp(8.0));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 53, 8, -22528, -256 },
				};

				assert_equal(reference2, cells);

				// INIT
				cells.clear();

				// ACT
				vr.line(fp(-3000.75), fp(-9.8), fp(-3000.75), fp(-10.0));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference3[] = {
					{ -3001, -10, -6528, -51 },
				};

				assert_equal(reference3, cells);
			}


			test( InPixelRectsAreProperlyRasterizedToCells )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

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

				assert_equal(reference1, cells);

				// INIT
				cells.clear();

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

				assert_equal(reference2, cells);
			}


			test( TwoRectsInPixelAreProperlyRasterizedToCells )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

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

				assert_equal(reference, cells);
			}


			test( CoverAndAreaAreAddedOnInterCellLineContinuation )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

				// ACT
				vr.line(fp(151.3), fp(11.1), fp(151.3), fp(11.2));
				vr.line(fp(151.3), fp(11.2), fp(151.3), fp(12.7));
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 151, 11, 2 * 77 * 25 + 2 * 77 * 205, 25 + 205 },
					{ 151, 12, 2 * 77 * 179, 179 },
				};

				assert_equal(reference, cells);
			}


			test( IntraCellPositivelyInclinedVectorsProducePositiveCoverAndArea )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

				// ACT
				vr.line(fp(0.83), fp(0.42), fp(0.14), fp(0.71)); // in fp - [(212, 108); (36, 182)), v(-176, 74)
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 0, 0, (212 + 36) * 74, 74 },
				};

				assert_equal(reference1, cells);

				// INIT
				cells.clear();

				// ACT
				vr.line(fp(-3.93), fp(-17000.7), fp(-3.428), fp(-17000.3)); // in fp - [(-1006, -4352179); (-878, -4352077)), v(128, 102)
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ -4, -17001, (18 + 146) * 102, 102 },
				};

				assert_equal(reference2, cells);
			}


			test( IntraCellLinesCoversAndAreasAreAdditive )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

				// ACT (right triangle)
				vr.line(fp(0.0), fp(0.0), fp(0.5), fp(0.0));	// area: 0, cover: 0
				vr.line(fp(0.5), fp(0.0), fp(0.5), fp(0.5)); // area: 32768, cover: 128
				vr.line(fp(0.5), fp(0.5), fp(0.0), fp(0.0)); // area: -16384, cover: -128
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 0, 0, 16384, 0 },
				};

				assert_equal(reference, cells);
			}


			test( DeltaReminderIsAddedToTheLastCell )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

				// ACT (tg = 2447)
				vr.line(fp(1.34), fp(10.13), fp(2.0), fp(10.523)); // in fp - [(343, 2593); (512, 2694)), v(169, 101)
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 1, 10, 34300, 100 },
					{ 2, 10, 0, 1 },
				};

				assert_equal(reference, cells);
			}


			test( InterCellPositivelyInclinedHLineProducePositiveCoverAndArea )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

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

				assert_equal(reference1, cells);

				// INIT
				cells.clear();

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

				assert_equal(reference2, cells);
			}


			test( InterCellNegativelyInclinedHLineProduceNegativeCoverAndArea )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

				// ACT
				vr.line(fp(5.43), fp(130.523), fp(1.34), fp(130.13)); // in fp - [(1390, 33414); (343, 33313)), v(-1047, -101)
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 5, 130, -1100, -10 },
					{ 4, 130, -6400, -25},
					{ 3, 130, -6144, -24 },
					{ 2, 130, -6400, -25 },
					{ 1, 130, -5831, -17 },
				};

				assert_equal(reference, cells);
			}


			test( InterCellLowRisingLineRepresentedBySparsedCells )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

				// ACT
				vr.line(0, 0, 1536, 3);
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 1, 0, 256, 1 },
					{ 3, 0, 256, 1 },
					{ 5, 0, 256, 1 },
				};

				assert_equal(reference, cells);
			}


			test( InterCellHLineSpanningFrom0To254 )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

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

				assert_equal(reference, cells);
			}


			test( InclinedIntraCellLinesCoversAndAreasAreAdditiveOnJoin )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

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

				assert_equal(reference, cells);
			}


			test( InterCellPositivelyInclinedTwoShortHLinesProducePositiveCoverAndArea )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

				// ACT (ctg = 252)
				vr.line(fp(1.2), fp(-1.9), fp(1.3), fp(-0.25)); // in fp - [(307, -486); (333, -64)), v(26, 422)
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference1[] = {
					{ 1, -2, 26680, 230 }, // dx = 14
					{ 1, -1, 27264, 192 }, // dx = 12
				};

				assert_equal(reference1, cells);

				// INIT
				cells.clear();

				// ACT (ctg = 3319)
				vr.line(fp(0.2), fp(1.9), fp(0.5), fp(2.27)); // in fp - [(51, 486); (128, 581)), v(77, 95)
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference2[] = {
					{ 0, 1, 3198, 26 }, // dx = 21
					{ 0, 2, 13800, 69 }, // dx = 56
				};

				assert_equal(reference2, cells);
			}


			test( InterCellNegativelyInclinedTwoShortHLinesProducePositiveCoverAndArea )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

				// ACT (ctg = -713)
				vr.line(fp(1.2), fp(1.9), fp(1.5), fp(0.17)); // in fp - [(307, 486); (384, 44)), v(77, -442)
				vr.commit();

				// ASSERT
				const vector_rasterizer::cell reference[] = {
					{ 1, 1, -32660, -230 }, // dx = 40
					{ 1, 0, -46428, -212 }, // dx = 37
				};

				assert_equal(reference, cells);
			}


			test( InterCellPositivelyInclinedMultipleShortHLinesProducePositiveCoverAndArea )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

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

				assert_equal(reference, cells);
			}


			test( InterCellPositivelyInclinedMultipleShortHLinesNearBoundsProducePositiveCoverAndArea )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

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

				assert_equal(reference, cells);
			}


			test( InclinedInterCellShortHLinesCoversAndAreasAreAdditiveOnJoin )
			{
				// INIT
				vector_rasterizer::cells_container cells;
				vector_rasterizer vr(cells);

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

				assert_equal(reference, cells);
			}

		end_test_suite
	}
}
