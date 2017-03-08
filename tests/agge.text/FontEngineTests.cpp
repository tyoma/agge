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
					e.create_font(L"arial", 13, false, false, font_engine::gf_strong),
					e.create_font(L"helvetica", 13, false, false, font_engine::gf_strong),
					e.create_font(L"tahoma", 13, false, false, font_engine::gf_strong),
					e.create_font(L"tahoma", 14, false, false, font_engine::gf_strong),
					e.create_font(L"helvetica", 13, true, false, font_engine::gf_strong),
					e.create_font(L"helvetica", 13, false, true, font_engine::gf_strong),
					e.create_font(L"helvetica", 13, true, true, font_engine::gf_strong),
					e.create_font(L"helvetica", 13, true, true, font_engine::gf_vertical),
					e.create_font(L"helvetica", 13, true, true, font_engine::gf_none),
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
					e.create_font(L"arial", 13, false, false, font_engine::gf_strong),
					e.create_font(L"ARial", 13, false, false, font_engine::gf_strong),
					e.create_font(L"arial", 15, false, false, font_engine::gf_vertical),
					e.create_font(L"Arial", 15, false, false, font_engine::gf_vertical),
					e.create_font(L"arial", 13, false, true, font_engine::gf_none),
					e.create_font(L"arial", 13, false, true, font_engine::gf_none),
					e.create_font(L"tahoma", 143, true, false, font_engine::gf_strong),
					e.create_font(L"taHOMA", 143, true, false, font_engine::gf_strong),
				};

				// ASSERT
				assert_equal(fonts[0], fonts[1]);
				assert_equal(fonts[2], fonts[3]);
				assert_equal(fonts[4], fonts[5]);
				assert_equal(fonts[6], fonts[7]);

				// ACT
				font::ptr f2 = e.create_font(L"arial", 13, false, false, font_engine::gf_strong);

				// ASSERT
				assert_equal(fonts[0], f2);
			}


			test( CreatingTheFontCreatesFontAccessor )
			{
				// INIT
				mocks::fonts_loader loader;
				font_engine e(loader);

				// ACT
				font::ptr fonts1[] = {
					e.create_font(L"arial", 13, false, false, font_engine::gf_strong),
					e.create_font(L"tahoma", 29, true, false, font_engine::gf_vertical),
					e.create_font(L"helvetica", 15, false, true, font_engine::gf_none /* freely scalable */),
				};
				fonts1;

				// ASSERT
				mocks::font_descriptor fd1[] = {
					{ L"arial", 13, false, false, font_engine::gf_strong },
					{ L"tahoma", 29, true, false, font_engine::gf_vertical },
					{ L"helvetica", 1000, false, true, font_engine::gf_none },
				};

				assert_equal(fd1, loader.created_log);

				// INIT
				loader.created_log.clear();

				// ACT
				font::ptr fonts2[] = {
					e.create_font(L"arial", 15, true, true, font_engine::gf_none /* freely scalable */),
				};
				fonts2;

				// ASSERT
				mocks::font_descriptor fd2[] = {
					{ L"arial", 1000, true, true, font_engine::gf_none },
				};

				assert_equal(fd2, loader.created_log);
			}


			test( ScalableFontAccessorIsCreatedOnceForDifferentFontSizes )
			{
				// INIT
				mocks::fonts_loader loader;
				font_engine e(loader);

				// ACT
				font::ptr fonts1[] = {
					e.create_font(L"arial", 13, false, true, font_engine::gf_none),
					e.create_font(L"arial", 29, false, true, font_engine::gf_none),
					e.create_font(L"arial", 140, false, true, font_engine::gf_none),
				};
				fonts1;

				// ASSERT
				mocks::font_descriptor fd1[] = {
					{ L"arial", 1000, false, true, font_engine::gf_none },
				};

				assert_equal(fd1, loader.created_log);

				// ACT
				font::ptr fonts2[] = {
					e.create_font(L"times", 31, true, false, font_engine::gf_none),
					e.create_font(L"times", 91, true, false, font_engine::gf_none),
				};
				fonts2;

				// ASSERT
				mocks::font_descriptor fd2[] = {
					{ L"arial", 1000, false, true, font_engine::gf_none },
					{ L"times", 1000, true, false, font_engine::gf_none },
				};

				assert_equal(fd2, loader.created_log);
			}

		end_test_suite
	}
}
