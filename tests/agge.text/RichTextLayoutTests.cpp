#include <agge.text/layout.h>

#include "mocks.h"
#include "helpers.h"
#include "helpers_layout.h"

#include <agge.text/font_factory.h>
#include <map>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace mocks
		{
			class font_factory : public agge::font_factory
			{
			public:
				struct font_descriptor_less
				{
					bool operator ()(const font_descriptor &lhs, const font_descriptor &rhs) const
					{	return lhs.family < rhs.family ? true : rhs.family < lhs.family ? false : lhs.height < rhs.height;	}
				};

				typedef map<font_descriptor, shared_ptr<font>, font_descriptor_less> fonts_map;

			public:
				fonts_map fonts;

			private:
				virtual shared_ptr<font> create_font(const font_descriptor &descriptor)
				{
					fonts_map::const_iterator i = fonts.find(descriptor);

					assert_not_equal(fonts.end(), i);
					return i->second;
				}
			};
		}

		begin_test_suite( RichTextLayoutTests )

			mocks::font_factory factory;
			shared_ptr<font> arial, helvetica, segoe;

			init( InitFonts )
			{
				font_metrics m1 = { 10.0f, 2.0f, 2.0f };
				font_metrics m2 = { 14.7f, 3.0f, 1.0f };
				font_metrics m3 = { 9.0f, 1.0f, 1.0f };
				mocks::font_accessor::char_to_index indices[] = { { L' ', 0 }, { L'A', 1 }, { L'B', 2 }, { L'C', 3 }, { L'D', 4 }, };
				mocks::font_accessor::char_to_index indices2[] = { { L' ', 0 }, { L'A', 5 }, { L'B', 2 }, { L'C', 3 }, { L'D', 4 }, };
				mocks::font_accessor::glyph glyphs[] = {
					{ { 3, 0 } },
					{ { 5, 0 } },
					{ { 7, 0 } },
					{ { 11, 0 } },
					{ { 13, 0 } },
					{ { 21.15, 0 } },
				};

				arial = factory.fonts[font_descriptor::create("Arial", 13)] = mocks::create_font(m1, indices, glyphs);
				helvetica = factory.fonts[font_descriptor::create("Helvetica", 17)] = mocks::create_font(m2, indices2, glyphs);
				segoe = factory.fonts[font_descriptor::create("Segoe UI", 10)] = mocks::create_font(m3, indices, glyphs);
			}


			test( FontSpecifiedByAnnotationIsAssignedToTheRange )
			{
				// INIT
				layout l(factory);
				richtext_t text;

				text << modify_family("Arial") << modify_height(13) << L"ADB\n"
					<< modify_family("Helvetica") << modify_height(17) << L"AAAAB\n"
					<< modify_family("Segoe UI") << modify_height(10) << L"ACAA\n";

				// ACT
				l.process(text);

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f, 0.0f, plural + ref_glyph_run(arial, 0.0f, 0.0f, plural + 1 + 4 + 2))
					+ ref_text_line(0.0f, 28.7f, 0.0f, plural + ref_glyph_run(helvetica, 0.0f, 0.0f, plural + 5 + 5 + 5 + 5 + 2))
					+ ref_text_line(0.0f, 41.7f, 0.0f, plural + ref_glyph_run(segoe, 0.0f, 0.0f, plural + 1 + 3 + 1 + 1)),
					mkvector(l.begin(), l.end()));
			}


			test( MultipleFontsShareTheSameSingleTextLine )
			{
				// INIT
				layout l(factory);
				richtext_t text;

				text << modify_family("Arial") << modify_height(13) << L"ADB " // width: 28
					<< modify_family("Helvetica") << modify_height(17) << L"AA" // width: 42.3
					<< modify_family("Segoe UI") << modify_height(10) << L"AB"; // width: 12

				// ACT
				l.process(text);

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 10.0f /* should be 14.7 */, 0.0f, plural
						+ ref_glyph_run(arial, 0.0f, 0.0f, plural + 1 + 4 + 2 + 0)
						+ ref_glyph_run(helvetica, 28.0f, 0.0f, plural + 5 + 5)
						+ ref_glyph_run(segoe, 70.3f, 0.0f, plural + 1 + 2)),
					mkvector(l.begin(), l.end()));
			}


			test( RegularWordBreakMovesNextGlyphRunToStartOfString )
			{
				// INIT
				layout l(factory);
				richtext_t text;

				text << modify_family("Arial") << modify_height(13) << L"ADB " // width: 28
					<< modify_family("Helvetica") << modify_height(17) << L"AA BB" // width: 42.3 + 3 + 14
					<< modify_family("Segoe UI") << modify_height(10) << L"AB"; // width: 12

				l.set_width_limit(70.4f);

				// ACT
				l.process(text);

				// ASSERT
				assert_equal(plural
					+ ref_text_line(0.0f, 14.7f, 70.3f, plural
						+ ref_glyph_run(arial, 0.0f, 0.0f, plural + 1 + 4 + 2 + 0)
						+ ref_glyph_run(helvetica, 28.0f, 0.0f, plural + 5 + 5))
					+ ref_text_line(0.0f, 33.4f, 26.0f, plural
						+ ref_glyph_run(helvetica, 0.0f, 0.0f, plural + 2 + 2)
						+ ref_glyph_run(segoe, 14.0f, 0.0f, plural + 1 + 2)),
					mkvector(l.begin(), l.end()));
			}
		end_test_suite
	}
}
