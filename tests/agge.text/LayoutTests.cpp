#include <agge.text/layout.h>

#include "mocks.h"

#include <ut/assert.h>
#include <ut/test.h>

namespace agge
{
	namespace tests
	{
		begin_test_suite( LayoutTests )
			test( EmptyLayoutHasEmptyBox )
			{
				// INIT / ACT
				layout l(L"", 0);

				// ACT
				box_r box = l.get_box();

				// ASSERT
				assert_equal(0.0f, box.w);
				assert_equal(0.0f, box.h);
			}

		
			test( SingleLineUnboundLayoutBoxEqualsSumOfAdvances )
			{
				// INIT
				mocks::font::char_to_index indices1[] = { { L'A', 1 }, { L'B', 0 }, { L'Q', 0 }, };
				mocks::font::glyph glyphs[] = {
					{ { 13, 0 } },
					{ { 11, 0 } },
				};
				font::ptr f1(new mocks::font(indices1, glyphs));

				// INIT / ACT
				layout l1(L"A", f1);
				layout l2(L"AAB", f1);
				layout l3(L"BQA", f1);

				// ACT
				box_r box1 = l1.get_box();
				box_r box2 = l2.get_box();
				box_r box3 = l3.get_box();

				// ASSERT
				assert_equal(11.0f, box1.w);
				assert_equal(35.0f, box2.w);
				assert_equal(37.0f, box3.w);

				// INIT
				mocks::font::char_to_index indices2[] = { { L'A', 0 }, { L'B', 0 }, { L'Q', 0 }, };
				font::ptr f2(new mocks::font(indices2, glyphs));

				// INIT / ACT
				layout l4(L"A", f2);
				layout l5(L"ABQABQABQ", f2);

				// ACT
				box_r box4 = l4.get_box();
				box_r box5 = l5.get_box();

				// ASSERT
				assert_equal(13.0f, box4.w);
				assert_equal(117.0f, box5.w);
			}


			test( MultiLineUnboundLayoutBoxEqualsMaxOfSumOfAdvancesInEachRow )
			{
				// INIT
				mocks::font::char_to_index indices[] = { { L' ', 0 }, { L'A', 1 }, { L'B', 2 }, { L'C', 3 }, };
				mocks::font::glyph glyphs[] = {
					{ { 7.1, 0 } },
					{ { 11, 0 } },
					{ { 13, 0 } },
					{ { 17, 0 } },
				};
				font::ptr f(new mocks::font(indices, glyphs));

				// ACT
				layout l1(L"ABC CBA AB\nABB BBC", f);
				layout l2(L"AC CB\nA AB\nABB BBC\n", f);
				box_r box1 = l1.get_box();
				box_r box2 = l2.get_box();

				// ASSERT
				assert_equal(120.2f, box1.w);
				assert_equal(87.1f, box2.w);
			}

		
		end_test_suite
	}
}
