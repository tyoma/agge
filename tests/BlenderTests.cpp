#include <agge/blenders_simd.h>

#include <utee/ut/assert.h>
#include <utee/ut/test.h>

using namespace std;

namespace agge
{
	namespace simd
	{
		namespace tests
		{
			begin_test_suite( IntelSolidColorBlenderTests )
				test( BlendOpaqueSinglePixelAligned )
				{
					// INIT
					blender_solid_color r0(rgba(0, 0, 0)), r1(rgba(1, 0, 0)), r2(rgba(100, 0, 0)), r3(rgba(255, 0, 0));
					blender_solid_color g1(rgba(0, 1, 0)), g2(rgba(0, 100, 0)), g3(rgba(0, 255, 0));
					blender_solid_color b1(rgba(0, 0, 1)), b2(rgba(0, 0, 100)), b3(rgba(0, 0, 255));
				
					blender_solid_color::pixel quads[10][4] = { 0 };
					blender_solid_color::cover_type opaque[] = { 0xFF, 0, 0, 0 };

					// ACT
					r0(&quads[0][0], 0, 0, 1, opaque);
					r1(&quads[1][0], 0, 0, 1, opaque);
					r2(&quads[2][0], 0, 0, 1, opaque);
					r3(&quads[3][0], 0, 0, 1, opaque);
					g1(&quads[4][0], 0, 0, 1, opaque);
					g2(&quads[5][0], 0, 0, 1, opaque);
					g3(&quads[6][0], 0, 0, 1, opaque);
					b1(&quads[7][0], 0, 0, 1, opaque);
					b2(&quads[8][0], 0, 0, 1, opaque);
					b3(&quads[9][0], 0, 0, 1, opaque);
				
					// ASSERT
					blender_solid_color::pixel reference[10] = {
						{ 0, 0, 0, 0 },
						{ 1, 0, 0, 0 },
						{ 100, 0, 0, 0 },
						{ 255, 0, 0, 0 },
						{ 0, 1, 0, 0 },
						{ 0, 100, 0, 0 },
						{ 0, 255, 0, 0 },
						{ 0, 0, 1, 0 },
						{ 0, 0, 100, 0 },
						{ 0, 0, 255, 0 },
					};

					assert_equal(reference[0], quads[0][0]);
					assert_equal(reference[1], quads[1][0]);
					assert_equal(reference[2], quads[2][0]);
					assert_equal(reference[3], quads[3][0]);
					assert_equal(reference[4], quads[4][0]);
					assert_equal(reference[5], quads[5][0]);
					assert_equal(reference[6], quads[6][0]);
					assert_equal(reference[7], quads[7][0]);
					assert_equal(reference[8], quads[8][0]);
					assert_equal(reference[9], quads[9][0]);
				}
			end_test_suite
		}
	}
}
