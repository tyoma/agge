#include <agge/blenders.h>
#include <agge/pixel.h>

#include <ut/assert.h>
#include <ut/test.h>

namespace agge
{
	namespace tests
	{
		namespace mocks
		{
			struct order_grab { enum { G = 0, R = 1, A = 2, B = 3 }; };

			template <typename ReferencePixelT = pixel32>
			struct blender
			{
				typedef ReferencePixelT pixel;

				blender(const pixel &reference_, uint8_t alpha_)
					: reference(reference_), alpha(alpha_)
				{	}

				pixel reference;
				uint8_t alpha;
			};
		}

		begin_test_suite( BlendersTests )
			test( CheckTranspositionR8G8B8X8 )
			{
				// INIT / ACT
				blender_solid_color<mocks::blender<>, order_rgba> b1(1, 50, 150, 200);
				blender_solid_color<mocks::blender<>, order_rgba> b2(150, 71, 15, 211);

				// ASSERT
				assert_equal(1, b1.reference.components[0]);
				assert_equal(50, b1.reference.components[1]);
				assert_equal(150, b1.reference.components[2]);
				assert_equal(200, b1.reference.components[3]);
				assert_equal(200, b1.alpha);
				assert_equal(150, b2.reference.components[0]);
				assert_equal(71, b2.reference.components[1]);
				assert_equal(15, b2.reference.components[2]);
				assert_equal(211, b2.reference.components[3]);
				assert_equal(211, b2.alpha);
			}


			test( CheckCustomTranspositionG8R8X8B8 )
			{
				// INIT / ACT
				blender_solid_color<mocks::blender<>, mocks::order_grab> b1(1, 50, 150, 199);
				blender_solid_color<mocks::blender<>, mocks::order_grab> b2(150, 71, 15, 211);

				// ASSERT
				assert_equal(50, b1.reference.components[0]);
				assert_equal(1, b1.reference.components[1]);
				assert_equal(199, b1.reference.components[2]);
				assert_equal(150, b1.reference.components[3]);
				assert_equal(199, b1.alpha);
				assert_equal(71, b2.reference.components[0]);
				assert_equal(150, b2.reference.components[1]);
				assert_equal(211, b2.reference.components[2]);
				assert_equal(15, b2.reference.components[3]);
				assert_equal(211, b2.alpha);
			}

		end_test_suite
	}
}
