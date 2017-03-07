#include <agge.text/font_engine.h>

#include "helpers.h"
#include "mocks.h"

#include <algorithm>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		begin_test_suite( FontEngineTests )
			
			test( CreatingDifferentFontsReturnsDifferentObjects )
			{
				// INIT
				mocks::fonts_loader loader;
				font_engine e(loader);

				// ACT
				font::ptr fonts[] = {
					e.create_font(L"arial", 13, false, false),
					e.create_font(L"helvetica", 13, false, false),
					e.create_font(L"tahoma", 13, false, false),
					e.create_font(L"tahoma", 14, false, false),
					e.create_font(L"helvetica", 13, true, false),
					e.create_font(L"helvetica", 13, false, true),
					e.create_font(L"helvetica", 13, true, true),
				};

				// ASSERT
				sort(tests::begin(fonts), tests::end(fonts));

				assert_equal(tests::end(fonts), unique(tests::begin(fonts), tests::end(fonts)));
			}


			test( CreatingTheSameFontReturnsTheSameObject )
			{
				// INIT
				mocks::fonts_loader loader;
				font_engine e(loader);

				// ACT
				font::ptr fonts[] = {
					e.create_font(L"arial", 13, false, false),
					e.create_font(L"ARial", 13, false, false),
					e.create_font(L"arial", 15, false, false),
					e.create_font(L"Arial", 15, false, false),
					e.create_font(L"arial", 13, false, true),
					e.create_font(L"arial", 13, false, true),
					e.create_font(L"tahoma", 143, true, false),
					e.create_font(L"taHOMA", 143, true, false),
				};

				// ASSERT
				assert_equal(fonts[0], fonts[1]);
				assert_equal(fonts[2], fonts[3]);
				assert_equal(fonts[4], fonts[5]);
				assert_equal(fonts[6], fonts[7]);

				// ACT
				font::ptr f2 = e.create_font(L"arial", 13, false, false);

				// ASSERT
				assert_equal(fonts[0], f2);
			}

		end_test_suite
	}
}
